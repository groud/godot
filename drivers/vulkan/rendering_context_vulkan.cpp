/*************************************************************************/
/*  rendering_context_vulkan.cpp                                         */
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

#if defined(VULKAN_ENABLED)

#include "os/os.h"

#include "rendering_context_vulkan.h"

#include "project_settings.h"
#include "version_generated.gen.h"

#include "shaders/frag.h"
#include "shaders/vert.h"

#include "main/splash.gen.h" //TODO: remove

VkResult RenderingContextVulkan::_create_debug_report_callback_EXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugReportCallbackEXT *pCallback) {
	PFN_vkCreateDebugReportCallbackEXT func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
	if (func != NULL) {
		return func(instance, pCreateInfo, pAllocator, pCallback);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void RenderingContextVulkan::_destroy_debug_report_callback_EXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks *pAllocator) {
	PFN_vkDestroyDebugReportCallbackEXT func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
	if (func != NULL) {
		func(instance, callback, pAllocator);
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL RenderingContextVulkan::_debug_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char *layerPrefix, const char *msg, void *userData) {

	print_line("validation layer: " + String(msg));

	return VK_FALSE;
}

VkDebugReportCallbackEXT vulkan_debug_callback;
void RenderingContextVulkan::_enable_debug() {
	// Debug layer
	if (enable_validation) {
		VkDebugReportCallbackCreateInfoEXT callback_info = {};
		callback_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		callback_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
		callback_info.pfnCallback = _debug_callback;

		if (_create_debug_report_callback_EXT(instance, &callback_info, NULL, &vulkan_debug_callback) != VK_SUCCESS) {
			ERR_EXPLAIN("failed to set up debug callback!");
			ERR_FAIL();
		} else {
			print_line("ENABLED DEBUG !");
		}
	}
}

bool RenderingContextVulkan::_check_validation_layer_support(Vector<const char *> validationLayers) {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, NULL);
	Vector<VkLayerProperties> availableLayers;
	availableLayers.resize(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.ptrw());

	for (int i = 0; i < validationLayers.size(); i++) {
		bool layerFound = false;

		print_line("Available:");
		printf("%d\n", layerCount);
		for (int j = 0; j < availableLayers.size(); j++) {
			print_line("Available:" + String(availableLayers[j].layerName));
			if (strcmp(validationLayers[i], availableLayers[j].layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			ERR_EXPLAIN("Could not find extension " + String(validationLayers[i]))
			ERR_FAIL_V(false);
		}
	}

	return true;
}

Error RenderingContextVulkan::_create_instance() {
	// App infos
	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	const String name = ProjectSettings::get_singleton()->get("application/config/name");
	const String product_version = ProjectSettings::get_singleton()->get("application/product_version");
	const String application_name = name + " " + product_version;
	app_info.pApplicationName = "Godot"; //application_name.utf8().get_data(); -> this does not work
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0); // TODO: retreive Godot version ?
	app_info.pEngineName = VERSION_NAME;
#if defined(VERSION_PATCH)
	const int patch = VERSION_PATCH;
#else
	const int patch = 0;
#endif
	app_info.engineVersion = VK_MAKE_VERSION(VERSION_MAJOR, VERSION_MINOR, patch);
	app_info.apiVersion = VK_API_VERSION_1_1;

	// Instance creation infos
	VkInstanceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &app_info;
	create_info.flags = 0;

	// Get the required extensions
	Vector<const char *> extensions;
	if (enable_validation) {
		extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}
	extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	extensions.push_back(_get_surface_extension());
	extensions.resize(extensions.size());

	create_info.ppEnabledExtensionNames = extensions.ptr();
	create_info.enabledExtensionCount = extensions.size();

	// Enable the validation layers
	validation_layers = Vector<const char *>();
	if (enable_validation) {
		validation_layers.push_back("VK_LAYER_LUNARG_standard_validation");
		//"VK_LAYER_LUNARG_api_dump"
	}

	if (!_check_validation_layer_support(validation_layers)) {
		ERR_EXPLAIN("Validation layers requested, but not available!");
		ERR_FAIL_V(ERR_CANT_CREATE);
	};

	create_info.ppEnabledLayerNames = validation_layers.ptr();
	create_info.enabledLayerCount = validation_layers.size();

	if (vkCreateInstance(&create_info, NULL, &instance) != VK_SUCCESS) {
		ERR_EXPLAIN("Can't Create A Vulkan Instance.")
		ERR_FAIL_V(ERR_CANT_CREATE);
	}

	return OK;
}

bool RenderingContextVulkan::_is_swap_chain_adequate(const VkPhysicalDevice p_physical_device) {
	Set<CharString> device_extensions;
	device_extensions.insert(String(VK_KHR_SWAPCHAIN_EXTENSION_NAME).utf8());

	bool extensions_supported = _check_device_extension_support(p_physical_device, device_extensions);

	bool swapChainAdequate = false;
	if (extensions_supported) {
		SwapChainSupportDetails swapChainSupport = _query_swap_chain_support(p_physical_device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.present_modes.empty();
	}

	if (!swapChainAdequate) {
		return false;
	}

	return true;
}

void RenderingContextVulkan::_update_uniform_buffer(uint32_t currentImage) {
	CameraMatrix model;
	model.set_identity();
	CameraMatrix view;
	view.set_identity();
	CameraMatrix proj;
	proj.set_identity();
	//proj.set_orthogonal(-2.0, 2.0, 2.0, -2.0, -1000.0, 1000.0);

	// Build the MVP matrix
	UniformBufferObject ubo = { model, view, proj };
	ubo.model = model;
	ubo.view = view;
	ubo.proj = proj;

	// Copy the uniforms into the buffer
	void *data;
	vkMapMemory(device, uniform_buffers_memory[currentImage], 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(device, uniform_buffers_memory[currentImage]);
}

void RenderingContextVulkan::swap_buffers() {
	print_line("Drawing");

	vkWaitForFences(device, 1, &in_flight_fences[currentFrame], VK_TRUE, UINT64_MAX);

	// Get the next image
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(device, swap_chain, UINT64_MAX, image_available_semaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		_recreate_swap_chain();
		return;
	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		ERR_EXPLAIN("Failed to acquire swap chain image!");
		ERR_FAIL()
	}

	vkResetFences(device, 1, &in_flight_fences[currentFrame]);

	VkSemaphore signalSemaphores[] = { render_finished_semaphores[currentFrame] };

	_update_uniform_buffer(imageIndex);

	// Submit to the queue
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	VkSemaphore waitSemaphores[] = { image_available_semaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &command_buffers[imageIndex];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(graphics_queue, 1, &submitInfo, in_flight_fences[currentFrame]) != VK_SUCCESS) {
		ERR_EXPLAIN("Failed to submit draw command buffer!");
		ERR_FAIL()
	}

	// Present
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	VkSwapchainKHR swap_chains[] = { swap_chain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swap_chains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = NULL; // Optional

	result = vkQueuePresentKHR(present_queue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebuffer_resized) {
		framebuffer_resized = false;
		_recreate_swap_chain();
	} else if (result != VK_SUCCESS) {
		ERR_EXPLAIN("Failed to present swap chain image!");
		ERR_FAIL()
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void RenderingContextVulkan::set_use_vsync(bool p_use) {
	// TODO: Implement vsync
	use_vsync = p_use;
}

bool RenderingContextVulkan::is_using_vsync() const {

	return use_vsync;
}

bool RenderingContextVulkan::_check_device_extension_support(VkPhysicalDevice device, Set<CharString> device_extensions) {
	uint32_t extension_count;
	vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, NULL);

	Vector<VkExtensionProperties> available_extensions;
	available_extensions.resize(extension_count);
	vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, available_extensions.ptrw());

	Set<CharString> required_extensions;
	for (Set<CharString>::Element *E = device_extensions.front(); E; E = E->next()) {
		required_extensions.insert(E->get());
	}

	for (int i = 0; i < available_extensions.size(); i++) {
		required_extensions.erase(String(available_extensions[i].extensionName).utf8());
	}

	return required_extensions.size() == 0;
}

RenderingContextVulkan::SwapChainSupportDetails RenderingContextVulkan::_query_swap_chain_support(VkPhysicalDevice device) {
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t format_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, NULL);

	if (format_count != 0) {
		details.formats.resize(format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats.ptrw());
	}

	uint32_t present_mode_count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, NULL);

	if (present_mode_count != 0) {
		details.present_modes.resize(present_mode_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details.present_modes.ptrw());
	}

	return details;
}

VkSurfaceFormatKHR RenderingContextVulkan::_choose_swap_surface_format(const Vector<VkSurfaceFormatKHR> &available_formats) {
	if (available_formats.size() == 1 && available_formats[0].format == VK_FORMAT_UNDEFINED) {
		return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	}
	for (int i = 0; i < available_formats.size(); i++) {
		if (available_formats[i].format == VK_FORMAT_B8G8R8A8_UNORM && available_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return available_formats[i];
		}
	}

	return available_formats[0];
}

VkPresentModeKHR RenderingContextVulkan::_choose_swap_present_mode(const Vector<VkPresentModeKHR> available_present_modes) {
	VkPresentModeKHR best_mode = VK_PRESENT_MODE_FIFO_KHR;

	for (int i = 0; i < available_present_modes.size(); i++) {
		if (available_present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
			return available_present_modes[i];
		} else if (available_present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			best_mode = available_present_modes[i];
		}
	}

	return best_mode;
}

VkExtent2D RenderingContextVulkan::_choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities) {
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	} else {
		VkExtent2D actual_extent = { WIDTH, HEIGHT };

		actual_extent.width = MAX(capabilities.minImageExtent.width, MIN(capabilities.maxImageExtent.width, actual_extent.width));
		actual_extent.height = MAX(capabilities.minImageExtent.height, MIN(capabilities.maxImageExtent.height, actual_extent.height));

		return actual_extent;
	}
}

Error RenderingContextVulkan::_pick_physical_device() {
	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(instance, &device_count, NULL);
	ERR_EXPLAIN("Can't find GPUs with Vulkan support");
	ERR_FAIL_COND_V(device_count == 0, ERR_CANT_CREATE);

	Vector<VkPhysicalDevice> devices;
	devices.resize(device_count);
	vkEnumeratePhysicalDevices(instance, &device_count, devices.ptrw());

	// Take the best available GPU
	int max_score = -1;
	for (int i = 0; i < devices.size(); i++) {
		int score = _get_physical_device_score(devices[i]);
		if (score > max_score) {
			physical_device = devices[i];
		}
	}

	ERR_EXPLAIN("Can't find a suitable GPU");
	ERR_FAIL_COND_V(physical_device == VK_NULL_HANDLE, ERR_CANT_CREATE);

	glad_vk_version = gladLoaderLoadVulkan(instance, physical_device, NULL);
	ERR_EXPLAIN("Can't Re-load Vulkan Symbols With Physical Device.");
	ERR_FAIL_COND_V(!glad_vk_version, ERR_CANT_CREATE);

	return OK;
}

RenderingContextVulkan::~RenderingContextVulkan() {
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(device, render_finished_semaphores[i], NULL);
		vkDestroySemaphore(device, image_available_semaphores[i], NULL);
		vkDestroyFence(device, in_flight_fences[i], NULL);
	}

	vkDestroyCommandPool(device, command_pool, NULL);

	vkDestroyDevice(device, NULL);
	if (enable_validation) {
		_destroy_debug_report_callback_EXT(instance, vulkan_debug_callback, NULL);
	}
	vkDestroySurfaceKHR(instance, surface, NULL);
	vkDestroyInstance(instance, NULL);
	gladLoaderUnloadVulkan();
}

Error RenderingContextVulkan::_create_semaphores_and_fences() {
	image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
	render_finished_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
	in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(device, &semaphoreInfo, NULL, &(image_available_semaphores.ptrw()[i])) != VK_SUCCESS ||
				vkCreateSemaphore(device, &semaphoreInfo, NULL, &(render_finished_semaphores.ptrw()[i])) != VK_SUCCESS ||
				vkCreateFence(device, &fenceInfo, NULL, &(in_flight_fences.ptrw())[i]) != VK_SUCCESS) {
			ERR_EXPLAIN("Failed to create synchronization objects.");
			ERR_FAIL_V(ERR_CANT_CREATE);
		}
	}
	return OK;
}
void RenderingContextVulkan::_create_framebuffers() {
	swap_chain_framebuffers.resize(swap_chain_image_views.size());
	for (size_t i = 0; i < swap_chain_image_views.size(); i++) {
		VkImageView attachments[] = {
			swap_chain_image_views[i]
		};

		VkFramebufferCreateInfo framebuffer_info = {};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = render_pass;
		framebuffer_info.attachmentCount = 1;
		framebuffer_info.pAttachments = attachments;
		framebuffer_info.width = swap_chain_extent.width;
		framebuffer_info.height = swap_chain_extent.height;
		framebuffer_info.layers = 1;

		if (vkCreateFramebuffer(device, &framebuffer_info, NULL, &swap_chain_framebuffers.write[i]) != VK_SUCCESS) {
			ERR_EXPLAIN("Can't Create Framebuffer.");
			ERR_FAIL();
		}
	}
}

void RenderingContextVulkan::_create_render_pass() {
	VkAttachmentDescription color_attachment = {};
	color_attachment.format = swap_chain_image_format;
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref = {};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &color_attachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device, &renderPassInfo, NULL, &render_pass) != VK_SUCCESS) {
		ERR_EXPLAIN("Can't Create Render Pass.");
		ERR_FAIL();
	}
}

Error RenderingContextVulkan::_create_image_views() {
	swap_chain_image_views.resize(swap_chain_images.size());
	for (size_t i = 0; i < swap_chain_images.size(); i++) {
		VkImageViewCreateInfo create_info = {};
		create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		create_info.image = swap_chain_images[i];

		create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		create_info.format = swap_chain_image_format;
		create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		create_info.subresourceRange.baseMipLevel = 0;
		create_info.subresourceRange.levelCount = 1;
		create_info.subresourceRange.baseArrayLayer = 0;
		create_info.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &create_info, NULL, &swap_chain_image_views.write[i]) != VK_SUCCESS) {
			ERR_EXPLAIN("Can't Create Vulkan Image Views.");
			ERR_FAIL_V(ERR_CANT_CREATE);
		}
	}

	return OK;
}

void RenderingContextVulkan::_cleanup_swap_chain() {

	//vkDestroyImageView(device, depth_image_view, NULL);
	//vkDestroyImage(device, depth_image, NULL);
	//vkFreeMemory(device, depth_image_memory, NULL);

	for (VkFramebuffer framebuffer : swap_chain_framebuffers) {
		vkDestroyFramebuffer(device, framebuffer, NULL);
	}

	vkFreeCommandBuffers(device, command_pool, static_cast<uint32_t>(command_buffers.size()), command_buffers.ptr());

	vkDestroyRenderPass(device, render_pass, NULL);

	for (VkImageView imageView : swap_chain_image_views) {
		vkDestroyImageView(device, imageView, NULL);
	}
	vkDestroySwapchainKHR(device, swap_chain, NULL);
}

void RenderingContextVulkan::cleanup() {
	_cleanup_swap_chain();

	vkDestroySampler(device, texture_sampler, NULL);
	vkDestroyImageView(device, texture_image_view, NULL);
	vkDestroyImage(device, texture_image, NULL);
	vkFreeMemory(device, texture_image_memory, NULL);

	vkDestroyDescriptorPool(device, descriptor_pool, NULL);
	vkDestroyDescriptorSetLayout(device, descriptor_set_layout, NULL);

	for (size_t i = 0; i < swap_chain_images.size(); i++) {
		vkDestroyBuffer(device, uniform_buffers[i], NULL);
		vkFreeMemory(device, uniform_buffers_memory[i], NULL);
	}
	vkDestroyBuffer(device, vertex_buffer, NULL);
	vkFreeMemory(device, vertex_buffer_memory, NULL);
	vkDestroyBuffer(device, index_buffer, NULL);
	vkFreeMemory(device, index_buffer_memory, NULL);

	vkDestroyPipeline(device, graphics_pipeline, NULL);
	vkDestroyPipelineLayout(device, pipeline_layout, NULL);
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(device, render_finished_semaphores[i], NULL);
		vkDestroySemaphore(device, image_available_semaphores[i], NULL);
		vkDestroyFence(device, in_flight_fences[i], NULL);
	}

	vkDestroyCommandPool(device, command_pool, NULL);

	vkDestroyDevice(device, NULL);
	if (enable_validation) {
		_destroy_debug_report_callback_EXT(instance, vulkan_debug_callback, NULL);
	}
	vkDestroySurfaceKHR(instance, surface, NULL);
	vkDestroyInstance(instance, NULL);
};

Error RenderingContextVulkan::_create_swap_chain() {
	SwapChainSupportDetails swap_chain_support = _query_swap_chain_support(physical_device);

	VkSurfaceFormatKHR surface_format = _choose_swap_surface_format(swap_chain_support.formats);
	VkPresentModeKHR present_mode = _choose_swap_present_mode(swap_chain_support.present_modes);
	VkExtent2D extent = _choose_swap_extent(swap_chain_support.capabilities);

	uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
	if (swap_chain_support.capabilities.maxImageCount > 0 && image_count > swap_chain_support.capabilities.maxImageCount) {
		image_count = swap_chain_support.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = surface;

	create_info.minImageCount = image_count;
	create_info.imageFormat = surface_format.format;
	create_info.imageColorSpace = surface_format.colorSpace;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = _pick_queue_families(physical_device);
	uint32_t queue_family_indices[] = { (uint32_t)indices.graphics_family, (uint32_t)indices.present_family };

	if (indices.graphics_family != indices.present_family) {
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices = queue_family_indices;
	} else {
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	create_info.preTransform = swap_chain_support.capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = present_mode;
	create_info.clipped = VK_TRUE;

	create_info.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device, &create_info, NULL, &swap_chain) != VK_SUCCESS) {
		ERR_EXPLAIN("Can't Create A Vulkan Swap Chain.")
		ERR_FAIL_V(ERR_CANT_CREATE);
	}

	vkGetSwapchainImagesKHR(device, swap_chain, &image_count, NULL);
	swap_chain_images.resize(image_count);
	vkGetSwapchainImagesKHR(device, swap_chain, &image_count, swap_chain_images.ptrw());

	swap_chain_image_format = surface_format.format;
	swap_chain_extent = extent;

	return OK;
}

void RenderingContextVulkan::_recreate_swap_chain() {

	/*int width = 0, height = 0;
  while (width == 0 || height == 0) {
    XEvent e;
    XNextEvent(x11_display, &e);
    if (e.type == ConfigureNotify) {
      XConfigureEvent xce = e.xconfigure;
      width = xce.width;
      height = xce.height;
    }

  }

  vkDeviceWaitIdle(device);
  */
	_cleanup_swap_chain();

	_create_swap_chain();
	_create_image_views();
	_create_render_pass();

	_create_graphic_pipeline();
	//_create_depth_resources();
	_create_framebuffers();
	_create_command_buffers();
}

Error RenderingContextVulkan::_create_logical_device() {
	QueueFamilyIndices indices = _pick_queue_families(physical_device);

	// Quere creation info
	float queue_priority = 1.0f;

	Vector<VkDeviceQueueCreateInfo> queue_create_infos;
	Set<int> unique_queue_families;
	unique_queue_families.insert(indices.graphics_family);
	unique_queue_families.insert(indices.present_family);

	for (Set<int>::Element *E = unique_queue_families.front(); E; E = E->next()) {
		VkDeviceQueueCreateInfo queue_create_info = {};
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex = E->get();
		queue_create_info.queueCount = 1;
		queue_create_info.pQueuePriorities = &queue_priority;
		queue_create_infos.push_back(queue_create_info);
	}

	// List of required device features
	VkPhysicalDeviceFeatures device_features = {};
	device_features.samplerAnisotropy = VK_TRUE;

	// Logical device creation
	VkDeviceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.pQueueCreateInfos = queue_create_infos.ptr();
	create_info.queueCreateInfoCount = queue_create_infos.size();
	create_info.pEnabledFeatures = &device_features;
	Vector<const char *> device_extensions;
	device_extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	create_info.enabledExtensionCount = device_extensions.size();
	create_info.ppEnabledExtensionNames = device_extensions.ptr();
	if (enable_validation) {
		create_info.ppEnabledLayerNames = validation_layers.ptr();
		create_info.enabledLayerCount = validation_layers.size();
	} else {
		create_info.enabledLayerCount = 0;
	}

	if (vkCreateDevice(physical_device, &create_info, NULL, &device) != VK_SUCCESS) {
		ERR_EXPLAIN("Can't Create A Vulkan Logical Device.");
		ERR_FAIL_V(ERR_CANT_CREATE);
	}

	// Load vulkan
	glad_vk_version = gladLoaderLoadVulkan(instance, physical_device, device);
	if (!glad_vk_version) {
		ERR_EXPLAIN("Can't Re-load Vulkan Symbols With Device.");
		ERR_FAIL_V(ERR_CANT_CREATE);
	}

	vkGetDeviceQueue(device, indices.graphics_family, 0, &graphics_queue);
	vkGetDeviceQueue(device, indices.present_family, 0, &present_queue);

	return OK;
}

int RenderingContextVulkan::_get_physical_device_score(const VkPhysicalDevice p_physical_device) {
	VkPhysicalDeviceProperties device_properties;
	VkPhysicalDeviceFeatures device_features;
	vkGetPhysicalDeviceProperties(p_physical_device, &device_properties);
	vkGetPhysicalDeviceFeatures(p_physical_device, &device_features);

	int score = 0;
	// Device type
	if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
		score = 1;
	} else if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
		score = 0;
	} else {
		return -1;
	}

	// Geometry shader
	if (!device_features.geometryShader)
		return -1;

	if (!_pick_queue_families(p_physical_device).is_complete())
		return -1;

	if (!_is_swap_chain_adequate(p_physical_device)) {
		return -1;
	}

	// Anisotropy support
	if (!device_features.samplerAnisotropy)
		return -1;

	return score;
}

struct RenderingContextVulkan::QueueFamilyIndices RenderingContextVulkan::_pick_queue_families(VkPhysicalDevice p_physical_device) {
	struct QueueFamilyIndices indices;
	// Family count and types
	uint32_t queue_family_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(p_physical_device, &queue_family_count, NULL);
	Vector<VkQueueFamilyProperties> queue_families;
	queue_families.resize(queue_family_count);
	vkGetPhysicalDeviceQueueFamilyProperties(p_physical_device, &queue_family_count, queue_families.ptrw());

	for (int i = 0; i < queue_families.size(); i++) {
		if (queue_families[i].queueCount > 0 && queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphics_family = i;
		}

		VkBool32 present_support = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(p_physical_device, i, surface, &present_support);

		if (queue_families[i].queueCount > 0 && present_support) {
			indices.present_family = i;
		}

		if (indices.is_complete()) {
			break;
		}
	}
	return indices;
}

VkCommandBuffer RenderingContextVulkan::_begin_single_time_commands() {
	// Allocate a new command buffer
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = command_pool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	// Start the command buffer
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void RenderingContextVulkan::_end_single_time_commands(VkCommandBuffer command_buffer) {
	// End command buffer
	vkEndCommandBuffer(command_buffer);

	// Execute the command buffer
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &command_buffer;

	vkQueueSubmit(graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphics_queue);

	// Free the command buffer
	vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
}

void RenderingContextVulkan::_copy_buffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
	// Copy a buffer into another
	VkCommandBuffer command_buffer = _begin_single_time_commands();

	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	vkCmdCopyBuffer(command_buffer, srcBuffer, dstBuffer, 1, &copyRegion);

	_end_single_time_commands(command_buffer);
}

void RenderingContextVulkan::_copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
	// Copy a buffer into an image
	VkCommandBuffer command_buffer = _begin_single_time_commands();

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	// The 4th parameters assumes the image has already been transitioned to an optimal layout for receiving data

	_end_single_time_commands(command_buffer);
}

void RenderingContextVulkan::_create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &buffer_memory) {
	// Create a vertex buffer (without any memory binded)
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferInfo, NULL, &buffer) != VK_SUCCESS) {
		ERR_EXPLAIN("Failed to create vertex buffer!");
		ERR_FAIL();
	}

	// Allocate the required memory
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = _find_memory_type(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &allocInfo, NULL, &buffer_memory) != VK_SUCCESS) {
		ERR_EXPLAIN("Failed to allocate vertex buffer memory!");
		ERR_FAIL();
	}

	// Bind the buffer to the memory
	vkBindBufferMemory(device, buffer, buffer_memory, 0);
}

void RenderingContextVulkan::_create_command_buffers() {

	// Allocate the command buffers
	command_buffers.resize(swap_chain_framebuffers.size());
	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = (uint32_t)command_buffers.size();

	if (vkAllocateCommandBuffers(device, &alloc_info, command_buffers.ptrw()) != VK_SUCCESS) {
		ERR_EXPLAIN("Can't allocate command buffers!");
		ERR_FAIL();
	}

	for (size_t i = 0; i < command_buffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = NULL; // Optional

		if (vkBeginCommandBuffer(command_buffers[i], &beginInfo) != VK_SUCCESS) {
			ERR_EXPLAIN("Can't Begin Recording Command Buffer.");
			ERR_FAIL();
		}

		// Fills in the render pass info
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = render_pass;
		renderPassInfo.framebuffer = swap_chain_framebuffers[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swap_chain_extent;
		Vector<VkClearValue> clearValues;
		clearValues.resize(2);
		clearValues.write[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues.write[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = clearValues.size();
		renderPassInfo.pClearValues = clearValues.ptr();

		// Fills in the command buffer
		vkCmdBeginRenderPass(command_buffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);
		VkBuffer vertex_buffers[] = { vertex_buffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(command_buffers[i], 0, 1, vertex_buffers, offsets);
		vkCmdBindIndexBuffer(command_buffers[i], index_buffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_sets[i], 0, NULL);
		vkCmdDrawIndexed(command_buffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
		vkCmdEndRenderPass(command_buffers[i]);

		if (vkEndCommandBuffer(command_buffers[i]) != VK_SUCCESS) {
			ERR_EXPLAIN("Can't Record Command Buffer!");
			ERR_FAIL();
		}
	}
}

void RenderingContextVulkan::_create_command_pool() {
	QueueFamilyIndices queueFamilyIndices = _pick_queue_families(physical_device);

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphics_family;
	poolInfo.flags = 0; // Optional

	if (vkCreateCommandPool(device, &poolInfo, NULL, &command_pool) != VK_SUCCESS) {
		ERR_EXPLAIN("Failed to create command pool!");
		ERR_FAIL();
	}
}

VkFormat RenderingContextVulkan::_find_supported_format(const Vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
	// Get acceptable image formats
	for (int i = 0; i < candidates.size(); i++) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physical_device, candidates[i], &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return candidates[i];
		} else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return candidates[i];
		}
	}
	ERR_EXPLAIN("Failed to find supported format!");
	ERR_FAIL_V(VkFormat());
}

VkFormat RenderingContextVulkan::_find_depth_format() {
	Vector<VkFormat> formats;
	formats.push_back(VK_FORMAT_D32_SFLOAT);
	formats.push_back(VK_FORMAT_D32_SFLOAT_S8_UINT);
	formats.push_back(VK_FORMAT_D24_UNORM_S8_UINT);

	return _find_supported_format(formats, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool RenderingContextVulkan::_has_stencil_component(VkFormat format) {
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

int RenderingContextVulkan::_create_depth_resources() {
	VkFormat depth_format = _find_depth_format();

	_create_image(swap_chain_extent.width, swap_chain_extent.height, 1, depth_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depth_image, depth_image_memory);

	depth_image_view = _create_image_view(depth_image, depth_format, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

	_transition_image_layout(depth_image, depth_format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);

	return 0;
}

void RenderingContextVulkan::_create_graphic_pipeline() {
	// Shaders
	Vector<uint8_t> vert_shader_code;
	vert_shader_code.resize(sizeof(___vert_spv));
	memcpy(vert_shader_code.ptrw(), ___vert_spv, sizeof(___vert_spv));
	Vector<uint8_t> frag_shader_code;
	frag_shader_code.resize(sizeof(___frag_spv));
	memcpy(frag_shader_code.ptrw(), ___frag_spv, sizeof(___frag_spv));
	VkShaderModule vertShaderModule = _create_shader_module(vert_shader_code);
	VkShaderModule fragShaderModule = _create_shader_module(frag_shader_code);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main"; // Entry point in the shader

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	// Pipeline
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	VkVertexInputBindingDescription binding_description = Vertex::get_binding_description();
	Vector<VkVertexInputAttributeDescription> attribute_descriptions = Vertex::get_attribute_descriptions();

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &binding_description;
	vertexInputInfo.pVertexAttributeDescriptions = attribute_descriptions.ptr();

	// How do we use vertex (here=triangle list)
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Viewport and scissors
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swap_chain_extent.width;
	viewport.height = (float)swap_chain_extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = swap_chain_extent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// Rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE; // TODOT CULL mode -> VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	// Multisampling (for anti aliasing)
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = NULL; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	// Color blending
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE; // No blending !
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	// Depth stencil
	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {}; // Optional

	// Pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptor_set_layout;
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = NULL; // Optional

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &pipeline_layout) != VK_SUCCESS) {
		ERR_EXPLAIN("Failed to create pipeline layout!");
		ERR_FAIL();
	}

	// Create the pipeline
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = NULL; // Optional
	pipelineInfo.layout = pipeline_layout;
	pipelineInfo.renderPass = render_pass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &graphics_pipeline) != VK_SUCCESS) {
		ERR_EXPLAIN("Failed to create graphics pipeline!");
		ERR_FAIL();
	}
	/*
  vkDestroyShaderModule(device, fragShaderModule, NULL);
  vkDestroyShaderModule(device, vertShaderModule, NULL);
  */
}

void RenderingContextVulkan::_create_descriptor_pool() {
	// Create descriptor pools
	Vector<VkDescriptorPoolSize> poolSizes;
	poolSizes.resize(2);
	poolSizes.write[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes.write[0].descriptorCount = static_cast<uint32_t>(swap_chain_images.size());
	poolSizes.write[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes.write[1].descriptorCount = static_cast<uint32_t>(swap_chain_images.size());

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.ptr();
	poolInfo.maxSets = static_cast<uint32_t>(swap_chain_images.size());

	if (vkCreateDescriptorPool(device, &poolInfo, NULL, &descriptor_pool) != VK_SUCCESS) {
		ERR_EXPLAIN("failed to create descriptor pool!");
		ERR_FAIL();
	}
}

void RenderingContextVulkan::_create_descriptor_set_layout() {
	// Create a descuiptor set for uniforms
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // Only for the vertex shader
	uboLayoutBinding.pImmutableSamplers = NULL; // Optional

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // Only for fragment shader
	samplerLayoutBinding.pImmutableSamplers = NULL;

	Vector<VkDescriptorSetLayoutBinding> bindings;
	bindings.push_back(uboLayoutBinding);
	bindings.push_back(samplerLayoutBinding);

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.ptr();

	if (vkCreateDescriptorSetLayout(device, &layoutInfo, NULL, &descriptor_set_layout) != VK_SUCCESS) {
		ERR_EXPLAIN("Failed to create descriptor set layout!");
		ERR_FAIL();
	}
}
void RenderingContextVulkan::_create_descriptor_sets() {
	// Create a descriptor set for each uniform buffer
	Vector<VkDescriptorSetLayout> layouts;
	for (int i = 0; i < swap_chain_images.size(); i++) {
		layouts.push_back(descriptor_set_layout);
	}

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptor_pool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(swap_chain_images.size());
	allocInfo.pSetLayouts = layouts.ptr();

	descriptor_sets.resize(swap_chain_images.size());
	if (vkAllocateDescriptorSets(device, &allocInfo, &descriptor_sets.write[0]) != VK_SUCCESS) {
		ERR_EXPLAIN("Failed to allocate descriptor sets!");
		ERR_FAIL();
	}

	for (size_t i = 0; i < swap_chain_images.size(); i++) {
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = uniform_buffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = texture_image_view;
		imageInfo.sampler = texture_sampler;

		Vector<VkWriteDescriptorSet> descriptor_writes;
		descriptor_writes.resize(2);

		descriptor_writes.write[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_writes.write[0].dstSet = descriptor_sets[i];
		descriptor_writes.write[0].dstBinding = 0;
		descriptor_writes.write[0].dstArrayElement = 0;
		descriptor_writes.write[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptor_writes.write[0].descriptorCount = 1;
		descriptor_writes.write[0].pBufferInfo = &bufferInfo;
		descriptor_writes.write[0].pNext = NULL;

		descriptor_writes.write[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_writes.write[1].dstSet = descriptor_sets[i];
		descriptor_writes.write[1].dstBinding = 1;
		descriptor_writes.write[1].dstArrayElement = 0;
		descriptor_writes.write[1].dstArrayElement = 0;
		descriptor_writes.write[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptor_writes.write[1].descriptorCount = 1;
		descriptor_writes.write[1].pImageInfo = &imageInfo;
		descriptor_writes.write[1].pNext = NULL;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.ptr(), 0, NULL);
	}
}

uint32_t RenderingContextVulkan::_find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physical_device, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	ERR_EXPLAIN("Failed to find suitable memory type!");
	ERR_FAIL_V(-1);
}

void RenderingContextVulkan::_transition_image_layout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mip_levels) {
	// Transition an image from one layout to another
	VkCommandBuffer command_buffer = _begin_single_time_commands();

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (_has_stencil_component(format)) {
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	} else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = mip_levels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	} else {
		ERR_EXPLAIN("Unsupported layout transition!");
		ERR_FAIL();
	}

	vkCmdPipelineBarrier(
			command_buffer,
			sourceStage, destinationStage,
			0,
			0, NULL,
			0, NULL,
			1, &barrier);

	_end_single_time_commands(command_buffer);
}

void RenderingContextVulkan::_create_image(uint32_t width, uint32_t height, uint32_t mip_levels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &image_memory) {
	// Create an image
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = mip_levels;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(device, &imageInfo, NULL, &image) != VK_SUCCESS) {
		ERR_EXPLAIN("Failed to create image!");
		ERR_FAIL();
	}

	// Allocate memory for the image
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = _find_memory_type(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &allocInfo, NULL, &image_memory) != VK_SUCCESS) {
		ERR_EXPLAIN("Failed to allocate image memory!");
		ERR_FAIL();
	}

	vkBindImageMemory(device, image, image_memory, 0);
}

VkImageView RenderingContextVulkan::_create_image_view(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mip_levels) {
	// Create an image view from an image
	VkImageViewCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	create_info.image = image;
	create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	create_info.format = format;
	create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	create_info.subresourceRange.aspectMask = aspectFlags;
	create_info.subresourceRange.baseMipLevel = 0;
	create_info.subresourceRange.levelCount = mip_levels;
	create_info.subresourceRange.baseArrayLayer = 0;
	create_info.subresourceRange.layerCount = 1;

	VkImageView image_view;
	if (vkCreateImageView(device, &create_info, NULL, &image_view) != VK_SUCCESS) {
		ERR_EXPLAIN("Failed to create image views!");
		ERR_FAIL_V(VkImageView());
	}

	return image_view;
}

int RenderingContextVulkan::_create_vertex_buffer() {
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	// Create a staging buffer (in host memory)
	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;
	_create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);

	// Fills the staging buffer
	void *data;
	vkMapMemory(device, staging_buffer_memory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.ptr(), (size_t)bufferSize);
	vkUnmapMemory(device, staging_buffer_memory);

	_create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertex_buffer, vertex_buffer_memory);

	// Copy the staging buffer into the vertex one (from host memory to GPU one)
	_copy_buffer(staging_buffer, vertex_buffer, bufferSize);

	// Free and destroy the staging buffer
	vkDestroyBuffer(device, staging_buffer, NULL);
	vkFreeMemory(device, staging_buffer_memory, NULL);
	return 0;
}

int RenderingContextVulkan::_create_index_buffer() {
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	// Create a staging buffer (in host memory)
	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;
	_create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);

	// Fills the staging buffer
	void *data;
	vkMapMemory(device, staging_buffer_memory, 0, bufferSize, 0, &data);
	memcpy(data, indices.ptr(), (size_t)bufferSize);
	vkUnmapMemory(device, staging_buffer_memory);

	_create_buffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, index_buffer, index_buffer_memory);

	// Copy the staging buffer into the vertex one (from host memory to GPU one)
	_copy_buffer(staging_buffer, index_buffer, bufferSize);

	// Free and destroy the staging buffer
	vkDestroyBuffer(device, staging_buffer, NULL);
	vkFreeMemory(device, staging_buffer_memory, NULL);
	return 0;
}

int RenderingContextVulkan::_create_uniform_buffers() {
	// Create a buffer for each image in the swap chain
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	uniform_buffers.resize(swap_chain_images.size());
	uniform_buffers_memory.resize(swap_chain_images.size());

	for (size_t i = 0; i < swap_chain_images.size(); i++) {
		_create_buffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniform_buffers.write[i], uniform_buffers_memory.write[i]);
	}

	return 0;
}

void RenderingContextVulkan::_create_mipmaps(VkImage image, VkFormat format, int32_t texWidth, int32_t texHeight, uint32_t mipLevels) {

	// Check if image format supports linear blitting
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(physical_device, format, &formatProperties);
	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
		ERR_EXPLAIN("Texture image format does not support linear blitting!");
		ERR_FAIL();
	}

	// Generate the mipmaps
	VkCommandBuffer commandBuffer = _begin_single_time_commands();

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;

	for (uint32_t i = 1; i < mipLevels; i++) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, NULL,
				0, NULL,
				1, &barrier);

		VkImageBlit blit = {};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(commandBuffer,
				image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, NULL,
				0, NULL,
				1, &barrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, NULL,
			0, NULL,
			1, &barrier);

	_end_single_time_commands(commandBuffer);
}

VkShaderModule RenderingContextVulkan::_create_shader_module(Vector<uint8_t> code) {
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t *>(code.ptr());

	VkShaderModule shader_module;
	if (vkCreateShaderModule(device, &createInfo, NULL, &shader_module) != VK_SUCCESS) {
		ERR_EXPLAIN("Failed to create shader module!");
		ERR_FAIL_V(VkShaderModule());
	}

	return shader_module;
}

void RenderingContextVulkan::_create_texture_image() {
	// Get image from file
	int texWidth, texHeight;

	//TO BE REPLACED:
	Ref<Image> boot_logo = memnew(Image(boot_splash_png));

	const unsigned char *pixels;

	if (boot_logo.is_valid()) {
		pixels = boot_logo->get_data().read().ptr();
	} else {
		ERR_EXPLAIN("Invalid boot logo!");
		ERR_FAIL();
	}
	//------

	texWidth = boot_logo->get_width();
	texHeight = boot_logo->get_height();

	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels) {
		ERR_EXPLAIN("Failed to load texture image!");
		ERR_FAIL();
	}
	mip_levels = static_cast<uint32_t>(floor(log(MAX(texWidth, texHeight)) / log(2.0f))) + 1;

	// Create staging buffer
	VkBuffer staging_buffer;
	VkDeviceMemory staging_buffer_memory;
	_create_buffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, staging_buffer, staging_buffer_memory);

	void *data;
	vkMapMemory(device, staging_buffer_memory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(device, staging_buffer_memory);

	// Create image
	_create_image(texWidth, texHeight, mip_levels, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, texture_image, texture_image_memory);

	// Transition the image to a layout optimal for receiving data
	_transition_image_layout(texture_image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mip_levels);

	// Copy the buffer to the image
	_copy_buffer_to_image(staging_buffer, texture_image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	// Transition the image to a layout optimal for
	//_transition_image_layout(texture_image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mip_levels);
	_create_mipmaps(texture_image, VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, mip_levels);

	vkDestroyBuffer(device, staging_buffer, NULL);
	vkFreeMemory(device, staging_buffer_memory, NULL);
}

void RenderingContextVulkan::_create_texture_image_view() {
	// Create an image view
	texture_image_view = _create_image_view(texture_image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, mip_levels);
}

void RenderingContextVulkan::_create_texture_sampler() {
	// Create a sampler to interpretate the texture texels
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	//  samplerInfo.minLod = static_cast<float>(mip_levels) / 2;
	samplerInfo.maxLod = static_cast<float>(mip_levels);

	if (vkCreateSampler(device, &samplerInfo, NULL, &texture_sampler) != VK_SUCCESS) {
		ERR_EXPLAIN("Failed to create texture sampler!");
		ERR_FAIL();
	}
}

void RenderingContextVulkan::_load_simple_quad() {
	vertices = Vector<Vertex>();
	vertices.push_back({ Vector2(-0.5f, -0.5f), Color(1.0f, 0.0f, 0.0f), Vector2(0.0f, 0.0f) });
	vertices.push_back({ Vector2(0.5f, -0.5f), Color(0.0f, 1.0f, 0.0f), Vector2(1.0f, 0.0f) });
	vertices.push_back({ Vector2(0.5f, 0.5f), Color(0.0f, 0.0f, 1.0f), Vector2(1.0f, 1.0f) });
	vertices.push_back({ Vector2(-0.5f, 0.5f), Color(1.0f, 1.0f, 1.0f), Vector2(0.0f, 1.0f) });

	indices = Vector<uint32_t>();
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(3);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(3);
}

void RenderingContextVulkan::release_current(){

};

void RenderingContextVulkan::make_current(){

};

Error RenderingContextVulkan::initialize() {
	if (OS::get_singleton()->is_stdout_verbose()) {
		print_line("Using Vulkan video driver");
	}

	int glad_vk_version = gladLoaderLoadVulkan(NULL, NULL, NULL);
	if (!glad_vk_version) {
		ERR_EXPLAIN("Unable to load Vulkan symbols: gladLoad failure.");
		ERR_FAIL_V(ERR_CANT_CREATE);
	}

	_create_window();
	print_line("Window");

	_create_instance();
	print_line("Instance");
	_enable_debug();
	print_line("Debug");
	_create_surface();
	print_line("Surface");
	_pick_physical_device();
	print_line("Physical device");
	_create_logical_device();
	print_line("Logical device");

	_create_swap_chain();
	print_line("Swap chain");
	_create_image_views();
	print_line("Image views");
	_create_render_pass();
	print_line("Render pass");

	_create_descriptor_set_layout();
	print_line("Descriptor set layout");

	_create_graphic_pipeline();
	print_line("Graphics pipeline");
	_create_command_pool();
	print_line("Command pool");
	_create_depth_resources();
	print_line("Depth resources");
	_create_framebuffers();
	print_line("Framebuffers");

	_create_texture_image();
	print_line("Texture image");
	_create_texture_image_view();
	print_line("texture image view");
	_create_texture_sampler();
	print_line("texture sampler");

	_load_simple_quad();
	print_line("Load quad");
	_create_vertex_buffer();
	print_line("Vertew buffer");
	_create_index_buffer();
	print_line("Index buffer");
	_create_uniform_buffers();
	print_line("Uniform buffer");

	_create_descriptor_pool();
	print_line("Descriptor pool");
	_create_descriptor_sets();
	print_line("Descriptor set");

	_create_command_buffers();
	print_line("Command buffers");
	_create_semaphores_and_fences();
	print_line("Semaphores and fences");
}
#endif
