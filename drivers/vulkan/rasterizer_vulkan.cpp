/*************************************************************************/
/*  rasterizer_vulkan.cpp                                                 */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2017 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2017 Godot Engine contributors (cf. AUTHORS.md)    */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#include "rasterizer_vulkan.h"

#include "os/os.h"
#include "project_settings.h"
#include <string.h>
RasterizerStorage *RasterizerVulkan::get_storage() {

	return storage;
}

RasterizerCanvas *RasterizerVulkan::get_canvas() {

	return canvas;
}

RasterizerScene *RasterizerVulkan::get_scene() {

	return scene;
}

void RasterizerVulkan::initialize() {

	if (OS::get_singleton()->is_stdout_verbose()) {
		print_line("Using Vulkan video driver");
	}

	storage->initialize();
	canvas->initialize();
	scene->initialize();
}

void RasterizerVulkan::begin_frame() {

	uint64_t tick = OS::get_singleton()->get_ticks_usec();

	double delta = double(tick - prev_ticks) / 1000000.0;
	delta *= Engine::get_singleton()->get_time_scale();

	time_total += delta;

	if (delta == 0) {
		//to avoid hiccups
		delta = 0.001;
	}

	prev_ticks = tick;

	double time_roll_over = GLOBAL_GET("rendering/limits/time/time_rollover_secs");
	if (time_total > time_roll_over)
		time_total = 0; //roll over every day (should be customz

	storage->frame.time[0] = time_total;
	storage->frame.time[1] = Math::fmod(time_total, 3600);
	storage->frame.time[2] = Math::fmod(time_total, 900);
	storage->frame.time[3] = Math::fmod(time_total, 60);
	storage->frame.count++;
	storage->frame.delta = delta;

	storage->frame.prev_tick = tick;

	storage->update_dirty_resources();

	storage->info.render_final = storage->info.render;
	storage->info.render.reset();

	scene->iteration();
}

void RasterizerVulkan::set_current_render_target(RID p_render_target) {

	if (!p_render_target.is_valid() && storage->frame.current_rt && storage->frame.clear_request) {
		//handle pending clear request, if the framebuffer was not cleared
		glBindFramebuffer(GL_FRAMEBUFFER, storage->frame.current_rt->fbo);
		print_line("unbind clear of: " + storage->frame.clear_request_color);
		glClearColor(
				storage->frame.clear_request_color.r,
				storage->frame.clear_request_color.g,
				storage->frame.clear_request_color.b,
				storage->frame.clear_request_color.a);

		glClear(GL_COLOR_BUFFER_BIT);
	}

	if (p_render_target.is_valid()) {
		RasterizerStorageVulkan::RenderTarget *rt = storage->render_target_owner.getornull(p_render_target);
		storage->frame.current_rt = rt;
		ERR_FAIL_COND(!rt);
		storage->frame.clear_request = false;

		glViewport(0, 0, rt->width, rt->height);

	} else {
		storage->frame.current_rt = NULL;
		storage->frame.clear_request = false;
		glViewport(0, 0, OS::get_singleton()->get_window_size().width, OS::get_singleton()->get_window_size().height);
		glBindFramebuffer(GL_FRAMEBUFFER, RasterizerStorageVulkan::system_fbo);
	}
}

void RasterizerVulkan::restore_render_target() {

	ERR_FAIL_COND(storage->frame.current_rt == NULL);
	RasterizerStorageVulkan::RenderTarget *rt = storage->frame.current_rt;
	glBindFramebuffer(GL_FRAMEBUFFER, rt->fbo);
	glViewport(0, 0, rt->width, rt->height);
}

void RasterizerVulkan::clear_render_target(const Color &p_color) {

	ERR_FAIL_COND(!storage->frame.current_rt);

	storage->frame.clear_request = true;
	storage->frame.clear_request_color = p_color;
}

void RasterizerVulkan::set_boot_image(const Ref<Image> &p_image, const Color &p_color, bool p_scale) {

	if (p_image.is_null() || p_image->empty())
		return;

	begin_frame();

	int window_w = OS::get_singleton()->get_video_mode(0).width;
	int window_h = OS::get_singleton()->get_video_mode(0).height;

	glBindFramebuffer(GL_FRAMEBUFFER, RasterizerStorageVulkan::system_fbo);
	glViewport(0, 0, window_w, window_h);
	glDisable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glClearColor(p_color.r, p_color.g, p_color.b, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	canvas->canvas_begin();

	RID texture = storage->texture_create();
	storage->texture_allocate(texture, p_image->get_width(), p_image->get_height(), p_image->get_format(), VS::TEXTURE_FLAG_FILTER);
	storage->texture_set_data(texture, p_image);

	Rect2 imgrect(0, 0, p_image->get_width(), p_image->get_height());
	Rect2 screenrect;
	if (p_scale) {

		if (window_w > window_h) {
			//scale horizontally
			screenrect.size.y = window_h;
			screenrect.size.x = imgrect.size.x * window_h / imgrect.size.y;
			screenrect.position.x = (window_w - screenrect.size.x) / 2;

		} else {
			//scale vertically
			screenrect.size.x = window_w;
			screenrect.size.y = imgrect.size.y * window_w / imgrect.size.x;
			screenrect.position.y = (window_h - screenrect.size.y) / 2;
		}
	} else {

		screenrect = imgrect;
		screenrect.position += ((Size2(window_w, window_h) - screenrect.size) / 2.0).floor();
	}

	RasterizerStorageVulkan::Texture *t = storage->texture_owner.get(texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, t->tex_id);
	canvas->draw_generic_textured_rect(screenrect, Rect2(0, 0, 1, 1));
	glBindTexture(GL_TEXTURE_2D, 0);
	canvas->canvas_end();

	storage->free(texture); // free since it's only one frame that stays there

	OS::get_singleton()->swap_buffers();
}

void RasterizerVulkan::blit_render_target_to_screen(RID p_render_target, const Rect2 &p_screen_rect, int p_screen) {

	ERR_FAIL_COND(storage->frame.current_rt);

	RasterizerStorageVulkan::RenderTarget *rt = storage->render_target_owner.getornull(p_render_target);
	ERR_FAIL_COND(!rt);

	canvas->canvas_begin();
	glDisable(GL_BLEND);
	glBindFramebuffer(GL_FRAMEBUFFER, RasterizerStorageVulkan::system_fbo);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, rt->color);
	//glBindTexture(GL_TEXTURE_2D, rt->effects.mip_maps[0].color);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, storage->resources.normal_tex);

	canvas->draw_generic_textured_rect(p_screen_rect, Rect2(0, 0, 1, -1));
	glBindTexture(GL_TEXTURE_2D, 0);
	canvas->canvas_end();
}

void RasterizerVulkan::end_frame(bool p_swap_buffers) {
	if (p_swap_buffers)
		OS::get_singleton()->swap_buffers();
	else
		glFinish();

}

void RasterizerVulkan::finalize() {

	storage->finalize();
	canvas->finalize();
}

Rasterizer *RasterizerVulkan::_create_current() {
	return memnew(RasterizerVulkan);
}

void RasterizerVulkan::make_current() {
	_create_func = _create_current;
}

void RasterizerVulkan::register_config() {

	/*GLOBAL_DEF("rendering/quality/filters/use_nearest_mipmap_filter", false);
	GLOBAL_DEF("rendering/quality/filters/anisotropic_filter_level", 4);
	ProjectSettings::get_singleton()->set_custom_property_info("rendering/quality/filters/anisotropic_filter_level", PropertyInfo(Variant::INT, "rendering/quality/filters/anisotropic_filter_level", PROPERTY_HINT_RANGE, "1,16,1"));
	GLOBAL_DEF("rendering/limits/time/time_rollover_secs", 3600);
        */
}

RasterizerVulkan::RasterizerVulkan() {

	storage = memnew(RasterizerStorageVulkan);
	canvas = memnew(RasterizerCanvasVulkan);
	scene = memnew(RasterizerSceneVulkan);
	canvas->storage = storage;
	canvas->scene_render = scene;
	storage->canvas = canvas;
	scene->storage = storage;
	storage->scene = scene;

	prev_ticks = 0;
	time_total = 0;
}

RasterizerVulkan::~RasterizerVulkan() {

	memdelete(storage);
	memdelete(canvas);
}
