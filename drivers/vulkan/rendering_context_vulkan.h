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

#include <windows.h>

#include "glad/vulkan.h"

#include "core/set.h"
#include "core/ustring.h"
#include "core/vector.h"
#include "servers/visual/rendering_context.h"
#include "typedefs.h"

class RenderingContextVulkan : public RenderingContext {
private:
	static RenderingContextVulkan *singleton;

	const int WIDTH = 800;
	const int HEIGHT = 600;

	HDC hDC;
	HGLRC hRC;
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
	VkSurfaceKHR surface;

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

protected:
	Set<CharString> device_extensions;
	Set<CharString> extensions;
	Set<CharString> validation_layers;
	RenderingContext *context;

public:
	static RenderingContextVulkan *get_singleton();

	virtual void release_current() = 0;

	virtual void make_current() = 0;

	virtual void swap_buffers() = 0;

	virtual Error initialize() = 0;

	virtual void set_use_vsync(bool p_use) = 0;
	virtual bool is_using_vsync() const = 0;

	RenderingContextVulkan();
	~RenderingContextVulkan();
};

//#endif

#endif
