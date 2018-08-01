/*************************************************************************/
/*  rendering_context.h                                                  */
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

#ifndef RENDERING_CONTEXT_VULKAN_H
#define RENDERING_CONTEXT_VULKAN_H

//#if defined(OPENGL_ENABLED) || defined(GLES_ENABLED)

#include "core/set.h"
#include "core/ustring.h"
#include "core/vector.h"
#include "servers/visual/rendering_context.h"
#include "typedefs.h"

#include "platform/windows/os_windows.h"

#include "glad/vulkan.h"
#include "thirdparty/shaderc/src/libshaderc/include/shaderc/shaderc.h"

class RenderingContextVulkan : public RenderingContext {
private:
	static RenderingContextVulkan *singleton;

	const int WIDTH = 800;
	const int HEIGHT = 600;

	unsigned int pixel_format;
	HWND hWnd;
	bool use_vsync;
	int glad_vk_version = 0;

	VkInstance instance;
	VkDebugReportCallbackEXT callback;
	VkDevice device;
	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	VkQueue graphics_queue;
	VkQueue present_queue;

	VkSwapchainKHR swap_chain;
	Vector<VkImage> swap_chain_images;
	VkFormat swap_chain_image_format;
	VkExtent2D swap_chain_extent;
	Vector<VkImageView> swap_chain_image_views;
	Vector<VkFramebuffer> swap_chain_framebuffers;

	VkRenderPass render_pass;
	VkPipelineLayout pipeline_layout;
	VkPipeline graphics_pipeline;

	VkCommandPool command_pool;
	Vector<VkCommandBuffer> command_buffers;

	VkSemaphore image_available_semaphore;
	VkSemaphore render_finished_semaphore;

	struct QueueFamilyIndices {
		int graphics_family = -1;
		int present_family = -1;

		bool is_complete() {
			return graphics_family >= 0 && present_family >= 0;
		}
	};

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		Vector<VkSurfaceFormatKHR> formats;
		Vector<VkPresentModeKHR> present_modes;
	};

protected:
	VkSurfaceKHR surface;

	Set<CharString> device_extensions;
	Set<CharString> extensions;
	Set<CharString> validation_layers;
	RenderingContext *context;

public:
	virtual void release_current() = 0;

	virtual void make_current() = 0;

	int get_window_width();
	int get_window_height();
	virtual void swap_buffers() = 0;

	virtual Error initialize() = 0;

	virtual void set_use_vsync(bool p_use) = 0;
	virtual bool is_using_vsync() const = 0;

	RenderingContextVulkan();
	bool check_device_extension_support(VkPhysicalDevice device);
	RenderingContextVulkan::SwapChainSupportDetails query_swap_chain_support(VkPhysicalDevice device);
	VkSurfaceFormatKHR choose_swap_surface_format(const Vector<VkSurfaceFormatKHR> &available_formats);
	VkPresentModeKHR choose_swap_present_mode(const Vector<VkPresentModeKHR> available_present_modes);
	VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities);
	Error pick_physical_device();
	Error create_instance();
	~RenderingContextVulkan();

public:
	void create_semaphores();
	void create_command_buffers();
	void create_command_pool();
	void create_render_pass();
	void create_framebuffers();
	void create_graphics_pipeline();

	Vector<uint8_t> compile_shader(const String shader, const String name, const shaderc_shader_kind kind);

	Error create_image_views();
	Error create_swap_chain();
	Error create_logical_device();
	int _get_physical_device_score(const VkPhysicalDevice p_physical_device);
	struct RenderingContextVulkan::QueueFamilyIndices _pick_queue_families(VkPhysicalDevice p_physical_device);
	bool is_swap_chain_adequate(const VkPhysicalDevice p_physical_device);

	Vector<VkCommandBuffer> &get_command_buffers() { return command_buffers; }
	VkSemaphore &get_image_available_semaphore() { return image_available_semaphore; }
	VkSemaphore &get_render_finished_semaphore() { return render_finished_semaphore; }

	VkDevice &get_device() { return device; }
	void set_device(VkDevice val) { device = val; }
	VkInstance &get_instance() { return instance; }
	void set_instance(VkInstance val) { instance = val; }
	VkSwapchainKHR &get_swap_chain() { return swap_chain; }
	void set_swap_chain(VkSwapchainKHR val) { swap_chain = val; }
	VkQueue &get_present_queue() { return present_queue; }
	void set_present_queue(VkQueue val) { present_queue = val; }
	VkQueue &get_graphics_queue() { return graphics_queue; }
	void set_graphics_queue(VkQueue val) { graphics_queue = val; }
};

#endif
