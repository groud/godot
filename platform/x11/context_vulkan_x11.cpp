
/*************************************************************************/
/*  context_vulkan.cpp                                                   */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2018 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2018 Godot Engine contributors (cf. AUTHORS.md)    */
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

#include "context_vulkan_x11.h"

//#if defined(OPENGL_ENABLED) || defined(GLES_ENABLED)

void RenderingContextVulkan_X11::release_current() {
}

void RenderingContextVulkan_X11::make_current() {
}

void RenderingContextVulkan_X11::swap_buffers() {
}

int RenderingContextVulkan_X11::get_window_width() {
}

int RenderingContextVulkan_X11::get_window_height() {
}

static void set_class_hint(Display *p_display, Window p_window) {
	XClassHint *classHint;

	/* set the name and class hints for the window manager to use */
	classHint = XAllocClassHint();
	if (classHint) {
		classHint->res_name = (char *)"Godot_Engine";
		classHint->res_class = (char *)"Godot";
	}
	XSetClassHint(p_display, p_window, classHint);
	XFree(classHint);
}

Error RenderingContextVulkan_X11::initialize() {

	// TODO: improve to use XCreateColormap, and determine best parameters instead of defaults ones

	int screen = DefaultScreen(x11_display);

	if (OS::get_singleton()->is_layered_allowed()) {

		XSetWindowAttributes swa;
		swa.colormap = XDefaultColormap(x11_display, screen);
		//XCreateColormap(x11_display, RootWindow(x11_display, screen), visual_info.visual, AllocNone);
		swa.border_pixel = 0;
		swa.background_pixmap = None;
		swa.background_pixel = 0;
		swa.border_pixmap = None;
		swa.event_mask = StructureNotifyMask;

		x11_window = XCreateWindow(x11_display, RootWindow(x11_display, screen), 0, 0, OS::get_singleton()->get_video_mode().width, OS::get_singleton()->get_video_mode().height, 0, CopyFromParent, InputOutput, CopyFromParent, CWBorderPixel | CWColormap | CWEventMask | CWBackPixel, &swa);

	} else {

		XSetWindowAttributes swa;
		swa.colormap = XDefaultColormap(x11_display, screen);
		//XCreateColormap(x11_display, RootWindow(x11_display, screen), visual_info.visual, AllocNone);
		swa.border_pixel = 0;
		swa.event_mask = StructureNotifyMask;

		x11_window = XCreateWindow(x11_display, RootWindow(x11_display, screen), 0, 0, OS::get_singleton()->get_video_mode().width, OS::get_singleton()->get_video_mode().height, 0, CopyFromParent, InputOutput, CopyFromParent, CWBorderPixel | CWColormap | CWEventMask, &swa);
	}

	ERR_FAIL_COND_V(!x11_window, ERR_UNCONFIGURED);
	set_class_hint(x11_display, x11_window);
	XMapWindow(x11_display, x11_window);

	XSync(x11_display, False);

	return OK;
}

void RenderingContextVulkan_X11::set_use_vsync(bool p_use) {
}

bool RenderingContextVulkan_X11::is_using_vsync() const {
}

RenderingContextVulkan_X11::RenderingContextVulkan_X11(::Display *p_x11_display, ::Window &p_x11_window, const OS::VideoMode &p_default_video_mode) :
		x11_window(p_x11_window) {

	default_video_mode = p_default_video_mode;
	x11_display = p_x11_display;
}

RenderingContextVulkan_X11::~RenderingContextVulkan_X11() {
}

//#endif
