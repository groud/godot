#ifndef RASTERIZER_VULKAN_H
#define RASTERIZER_VULKAN_H

#include "rasterizer_canvas_vulkan.h"
#include "rasterizer_scene_vulkan.h"
#include "rasterizer_storage_vulkan.h"
#include "servers/visual/rasterizer.h"

class RasterizerVulkan : public Rasterizer {
protected:
	RasterizerCanvasVulkan canvas;
	RasterizerStorageVulkan storage;
	RasterizerSceneVulkan scene;

public:
	RasterizerStorage *get_storage() { return &storage; }
	RasterizerCanvas *get_canvas() { return &canvas; }
	RasterizerScene *get_scene() { return &scene; }

	void set_boot_image(const Ref<Image> &p_image, const Color &p_color, bool p_scale) {}

	void initialize() {}
	void begin_frame(double frame_step) {}
	void set_current_render_target(RID p_render_target) {}
	void restore_render_target() {}
	void clear_render_target(const Color &p_color) {}
	void blit_render_target_to_screen(RID p_render_target, const Rect2 &p_screen_rect, int p_screen = 0) {}
	void end_frame(bool p_swap_buffers) {}
	void finalize() {}

	static Rasterizer *_create_current() {
		return memnew(RasterizerVulkan);
	}

	static void make_current() {
		_create_func = _create_current;
	}

	RasterizerVulkan() {}
	~RasterizerVulkan() {}
};

#endif // RASTERIZER_VULKAN_H
