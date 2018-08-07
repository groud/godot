#ifndef RASTERIZER_VULKAN_H
#define RASTERIZER_VULKAN_H

#if defined(VULKAN_ENABLED)

#include "core/project_settings.h"
#include "os/os.h"
#include "servers/visual/rasterizer.h"
#include "servers/visual/rendering_context.h"

#include "rasterizer_canvas_vulkan.h"
#include "rasterizer_scene_vulkan.h"
#include "rasterizer_storage_vulkan.h"

#include "glad/vulkan.h"

class RasterizerVulkan : public Rasterizer {

	// To be filled in
	VkInstance *instance;
	VkDevice *device;
	VkPhysicalDevice *physical_device = VK_NULL_HANDLE;
	VkQueue *graphics_queue;
	VkQueue *present_queue;
	VkSwapchainKHR *swap_chain;
	VkFormat *swap_chain_image_format;
	VkExtent2D *swap_chain_extent;
	Vector<VkFramebuffer> *swap_chain_framebuffers;
	// ------------------

protected:
	RasterizerCanvasVulkan canvas;
	RasterizerStorageVulkan storage;
	RasterizerSceneVulkan scene;

public:
	RasterizerStorage *get_storage() { return &storage; }
	RasterizerCanvas *get_canvas() { return &canvas; }
	RasterizerScene *get_scene() { return &scene; }

	void set_boot_image(const Ref<Image> &p_image, const Color &p_color, bool p_scale) {}

	void initialize();
	void begin_frame(double frame_step) {}
	void set_current_render_target(RID p_render_target) {}
	void restore_render_target() {}
	void clear_render_target(const Color &p_color) {}
	void blit_render_target_to_screen(RID p_render_target, const Rect2 &p_screen_rect, int p_screen = 0) {}
	void end_frame(bool p_swap_buffers);
	void finalize() {}

	static void make_current();

	static void register_config();

	RasterizerVulkan() {}
	~RasterizerVulkan();
};

#endif
#endif // RASTERIZER_VULKAN_H
