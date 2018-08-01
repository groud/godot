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

#include "os/os.h"
#include "project_settings.h"
#include "rendering_context_vulkan.h"
#include "shaders/frag.h"
#include "shaders/vert.h"
#include "version_generated.gen.h"
#include "os_windows.h"

#include <Windows.h>

#include "glad/vulkan.h"

#if defined(OPENGL_ENABLED) || defined(GLES_ENABLED)

RenderingContextVulkan::RenderingContextVulkan() {
}

bool RenderingContextVulkan::is_swap_chain_adequate(const VkPhysicalDevice p_physical_device) {
	bool extensions_supported = check_device_extension_support(p_physical_device);

	bool swapChainAdequate = false;
	if (extensions_supported) {
		SwapChainSupportDetails swapChainSupport = query_swap_chain_support(p_physical_device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.present_modes.empty();
	}

	if (!swapChainAdequate) {
		return false;
	}

	return true;
}

void RenderingContextVulkan::make_current() {
}

int RenderingContextVulkan::get_window_width() {

	return OS::get_singleton()->get_video_mode().width;
}

int RenderingContextVulkan::get_window_height() {

	return OS::get_singleton()->get_video_mode().height;
}

void RenderingContextVulkan::swap_buffers() {
}

void RenderingContextVulkan::set_use_vsync(bool p_use) {
	// TODO: Implement vsync
	use_vsync = p_use;
}

bool RenderingContextVulkan::is_using_vsync() const {

	return use_vsync;
}

bool RenderingContextVulkan::check_device_extension_support(VkPhysicalDevice device) {
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

RenderingContextVulkan::SwapChainSupportDetails RenderingContextVulkan::query_swap_chain_support(VkPhysicalDevice device) {
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

VkSurfaceFormatKHR RenderingContextVulkan::choose_swap_surface_format(const Vector<VkSurfaceFormatKHR> &available_formats) {
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

VkPresentModeKHR RenderingContextVulkan::choose_swap_present_mode(const Vector<VkPresentModeKHR> available_present_modes) {
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

VkExtent2D RenderingContextVulkan::choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities) {
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	} else {
		VkExtent2D actual_extent = { WIDTH, HEIGHT };

		actual_extent.width = MAX(capabilities.minImageExtent.width, MIN(capabilities.maxImageExtent.width, actual_extent.width));
		actual_extent.height = MAX(capabilities.minImageExtent.height, MIN(capabilities.maxImageExtent.height, actual_extent.height));

		return actual_extent;
	}
}

Error RenderingContextVulkan::pick_physical_device() {
	uint32_t device_count = 0;
	vkEnumeratePhysicalDevices(get_instance(), &device_count, NULL);
	if (device_count == 0) {
		ERR_FAIL_V(ERR_CANT_CREATE, "Can't find GPUs with Vulkan support");
	}

	Vector<VkPhysicalDevice> devices;
	devices.resize(device_count);
	vkEnumeratePhysicalDevices(get_instance(), &device_count, devices.ptrw());

	// Take the best available GPU
	int max_score = -1;
	for (int i = 0; i < devices.size(); i++) {
		int score = _get_physical_device_score(devices[i]);
		if (score > max_score) {
			physical_device = devices[i];
		}
	}

	if (physical_device == VK_NULL_HANDLE) {
		ERR_FAIL_V(ERR_CANT_CREATE, "Can't find a suitable GPU");
	}

	glad_vk_version = gladLoaderLoadVulkan(get_instance(), physical_device, NULL);
	if (!glad_vk_version) {
		ERR_FAIL_V(ERR_CANT_CREATE, "Can't Re-load Vulkan Symbols With Physical Device.");
	}

	return OK;
}

Error RenderingContextVulkan::create_instance() {
	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	const String name = ProjectSettings::get_singleton()->get("application/config/name");
	const String product_version = ProjectSettings::get_singleton()->get("application/product_version");
	const String application_name = name + " " + product_version;
	const CharString cstr = application_name.utf8();
	app_info.pApplicationName = cstr.get_data();
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = VERSION_NAME;
#if defined(VERSION_PATCH)
	const int patch = VERSION_PATCH;
#else
	const int patch = 0;
#endif
	app_info.engineVersion = VK_MAKE_VERSION(VERSION_MAJOR, VERSION_MINOR, patch);
	app_info.apiVersion = VK_API_VERSION_1_1;

	VkInstanceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.enabledLayerCount = 0;
	create_info.ppEnabledLayerNames = NULL;
	create_info.pApplicationInfo = &app_info;
	create_info.flags = 0;

	extensions.insert(String(VK_KHR_SURFACE_EXTENSION_NAME).utf8());
	extensions.insert(String(VK_KHR_WIN32_SURFACE_EXTENSION_NAME).utf8());
	Vector<const char *> e;
	for (Set<CharString>::Element *E = extensions.front(); E; E = E->next()) {
		e.push_back(E->get().ptr());
	}
	create_info.enabledExtensionCount = e.size();
	create_info.ppEnabledExtensionNames = e.ptr();

	create_info.enabledLayerCount = 0;

	if (vkCreateInstance(&create_info, NULL, &get_instance()) != VK_SUCCESS) {
		ERR_FAIL_V(ERR_CANT_CREATE, "Can't Create A Vulkan Instance.");
	}

	return OK;
}

RenderingContextVulkan::~RenderingContextVulkan() {
	vkDestroySemaphore(get_device(), get_render_finished_semaphore(), NULL);
	vkDestroySemaphore(get_device(), get_image_available_semaphore(), NULL);
	vkDestroyCommandPool(get_device(), command_pool, NULL);
	for (int i = 0; i < swap_chain_framebuffers.size(); i++) {
		vkDestroyFramebuffer(get_device(), swap_chain_framebuffers[i], NULL);
	}
	vkDestroyPipeline(get_device(), graphics_pipeline, NULL);
	vkDestroyPipelineLayout(get_device(), pipeline_layout, NULL);
	vkDestroyRenderPass(get_device(), render_pass, NULL);
	for (int i = 0; i < swap_chain_image_views.size(); i++) {
		vkDestroyImageView(get_device(), swap_chain_image_views[i], NULL);
	}
	vkDestroySwapchainKHR(get_device(), get_swap_chain(), NULL);
	vkDestroySurfaceKHR(get_instance(), surface, NULL);
	vkDestroyDevice(get_device(), NULL);
	vkDestroyInstance(get_instance(), NULL);
	gladLoaderUnloadVulkan();
}

void RenderingContextVulkan::create_semaphores() {
	VkSemaphoreCreateInfo semaphore_info = {};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (vkCreateSemaphore(get_device(), &semaphore_info, NULL, &get_image_available_semaphore()) != VK_SUCCESS ||
			vkCreateSemaphore(get_device(), &semaphore_info, NULL, &get_render_finished_semaphore()) != VK_SUCCESS) {

		ERR_FAIL("Can't Create Semaphores.");
	}
}

void RenderingContextVulkan::create_command_buffers() {
	get_command_buffers().resize(swap_chain_framebuffers.size());
	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = command_pool;
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = (uint32_t)get_command_buffers().size();

	if (vkAllocateCommandBuffers(get_device(), &alloc_info, get_command_buffers().ptrw()) != VK_SUCCESS) {
		ERR_FAIL("Can't allocate command buffers!");
	}

	for (size_t i = 0; i < get_command_buffers().size(); i++) {
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = NULL; // Optional

		if (vkBeginCommandBuffer(get_command_buffers()[i], &beginInfo) != VK_SUCCESS) {
			ERR_FAIL("Can't Begin Recording Command Buffer.");
		}

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = render_pass;
		renderPassInfo.framebuffer = swap_chain_framebuffers[i];

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swap_chain_extent;

		VkClearValue clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clear_color;

		vkCmdBeginRenderPass(get_command_buffers()[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(get_command_buffers()[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphics_pipeline);
		vkCmdDraw(get_command_buffers()[i], 3, 1, 0, 0);
		vkCmdEndRenderPass(get_command_buffers()[i]);

		if (vkEndCommandBuffer(get_command_buffers()[i]) != VK_SUCCESS) {
			ERR_FAIL("Can't Record Command Buffer!");
		}
	}
}

void RenderingContextVulkan::create_command_pool() {
	QueueFamilyIndices queue_family_indices = _pick_queue_families(physical_device);

	VkCommandPoolCreateInfo pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.queueFamilyIndex = queue_family_indices.graphics_family;
	pool_info.flags = 0; // Optional

	if (vkCreateCommandPool(get_device(), &pool_info, NULL, &command_pool) != VK_SUCCESS) {
		ERR_FAIL("Can't Create Command Pool.");
	}
}

void RenderingContextVulkan::create_render_pass() {
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

	if (vkCreateRenderPass(get_device(), &renderPassInfo, NULL, &render_pass) != VK_SUCCESS) {
		ERR_FAIL("Can't Create Render Pass.");
	}
}

void RenderingContextVulkan::create_framebuffers() {
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

		if (vkCreateFramebuffer(get_device(), &framebuffer_info, NULL, &swap_chain_framebuffers.write[i]) != VK_SUCCESS) {
			ERR_FAIL("Can't Create Framebuffer.");
		}
	}
}

void RenderingContextVulkan::create_graphics_pipeline() {

	VkShaderModule vert_shader_module;
	VkShaderModule frag_shader_module;

	VkShaderModuleCreateInfo vert_create_info = {};
	vert_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vert_create_info.codeSize = ___vert_spv_size;
	vert_create_info.pCode = reinterpret_cast<const uint32_t *>(___vert_spv);

	if (vkCreateShaderModule(get_device(), &vert_create_info, NULL, &vert_shader_module) != VK_SUCCESS) {
		ERR_FAIL("Can't Create Shader Module");
	}

	VkShaderModuleCreateInfo frag_create_info = {};
	frag_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	frag_create_info.codeSize = ___frag_spv_size;
	frag_create_info.pCode = reinterpret_cast<const uint32_t *>(___frag_spv);

	if (vkCreateShaderModule(get_device(), &frag_create_info, NULL, &frag_shader_module) != VK_SUCCESS) {
		ERR_FAIL("Can't Create Shader Module");
	}

	VkPipelineShaderStageCreateInfo vert_shader_stage_info = {};
	vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;

	vert_shader_stage_info.module = vert_shader_module;
	vert_shader_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo frag_shader_stage_info = {};
	frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	frag_shader_stage_info.module = frag_shader_module;
	frag_shader_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo shader_stages[] = { vert_shader_stage_info, frag_shader_stage_info };

	VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_info.vertexBindingDescriptionCount = 0;
	vertex_input_info.pVertexBindingDescriptions = NULL; // Optional
	vertex_input_info.vertexAttributeDescriptionCount = 0;
	vertex_input_info.pVertexAttributeDescriptions = NULL; // Optional

	VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

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

	VkPipelineViewportStateCreateInfo viewport_state = {};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.pViewports = &viewport;
	viewport_state.scissorCount = 1;
	viewport_state.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;

	rasterizer.rasterizerDiscardEnable = VK_FALSE;

	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;

	rasterizer.lineWidth = 1.0f;

	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = NULL; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineColorBlendAttachmentState color_blend_attachment = {};
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable = VK_FALSE;
	color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	color_blend_attachment.blendEnable = VK_TRUE;
	color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo color_blending = {};
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = VK_LOGIC_OP_COPY; // Optional
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &color_blend_attachment;
	color_blending.blendConstants[0] = 0.0f; // Optional
	color_blending.blendConstants[1] = 0.0f; // Optional
	color_blending.blendConstants[2] = 0.0f; // Optional
	color_blending.blendConstants[3] = 0.0f; // Optional

	VkDynamicState dynamic_states[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamic_state = {};
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.dynamicStateCount = 2;
	dynamic_state.pDynamicStates = dynamic_states;

	VkPipelineLayoutCreateInfo pipeline_layout_info = {};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 0; // Optional
	pipeline_layout_info.pSetLayouts = NULL; // Optional
	pipeline_layout_info.pushConstantRangeCount = 0; // Optional
	pipeline_layout_info.pPushConstantRanges = NULL; // Optional

	if (vkCreatePipelineLayout(get_device(), &pipeline_layout_info, NULL, &pipeline_layout) != VK_SUCCESS) {
		ERR_FAIL("Can't Create Pipeline Layout.")
	}

	VkGraphicsPipelineCreateInfo pipeline_info = {};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shader_stages;
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pColorBlendState = &color_blending;
	pipeline_info.layout = pipeline_layout;
	pipeline_info.renderPass = render_pass;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(get_device(), VK_NULL_HANDLE, 1, &pipeline_info, NULL, &graphics_pipeline) != VK_SUCCESS) {
		ERR_FAIL("Can't Create Graphics Pipeline.");
	}

	vkDestroyShaderModule(get_device(), frag_shader_module, NULL);
	vkDestroyShaderModule(get_device(), vert_shader_module, NULL);
}

Error RenderingContextVulkan::create_image_views() {
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

		if (vkCreateImageView(get_device(), &create_info, NULL, &swap_chain_image_views.write[i]) != VK_SUCCESS) {
			ERR_FAIL_V(ERR_CANT_CREATE, "Can't Create Vulkan Image Views.");
		}
	}

	return OK;
}

Error RenderingContextVulkan::create_swap_chain() {
	SwapChainSupportDetails swap_chain_support = query_swap_chain_support(physical_device);

	VkSurfaceFormatKHR surface_format = choose_swap_surface_format(swap_chain_support.formats);
	VkPresentModeKHR present_mode = choose_swap_present_mode(swap_chain_support.present_modes);
	VkExtent2D extent = choose_swap_extent(swap_chain_support.capabilities);

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

	if (vkCreateSwapchainKHR(get_device(), &create_info, NULL, &get_swap_chain()) != VK_SUCCESS) {
		ERR_FAIL_V(ERR_CANT_CREATE, "Can't Create A Vulkan Swap Chain.")
	}

	vkGetSwapchainImagesKHR(get_device(), get_swap_chain(), &image_count, NULL);
	swap_chain_images.resize(image_count);
	vkGetSwapchainImagesKHR(get_device(), get_swap_chain(), &image_count, swap_chain_images.ptrw());

	swap_chain_image_format = surface_format.format;
	swap_chain_extent = extent;

	return OK;
}

Error RenderingContextVulkan::create_logical_device() {
	QueueFamilyIndices indices = _pick_queue_families(physical_device);

	Vector<VkDeviceQueueCreateInfo> queue_create_infos;
	Set<int> unique_queue_families;
	unique_queue_families.insert(indices.graphics_family);
	unique_queue_families.insert(indices.present_family);

	float queue_priority = 1.0f;
	for (Set<int>::Element *E = unique_queue_families.front(); E; E = E->next()) {
		VkDeviceQueueCreateInfo queue_create_info = {};
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex = E->get();
		queue_create_info.queueCount = 1;
		queue_create_info.pQueuePriorities = &queue_priority;
		queue_create_infos.push_back(queue_create_info);
	}

	VkPhysicalDeviceFeatures device_features = {};

	VkDeviceCreateInfo create_info = {};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	create_info.queueCreateInfoCount = queue_create_infos.size();
	create_info.pQueueCreateInfos = queue_create_infos.ptr();

	create_info.pEnabledFeatures = &device_features;

	Vector<const char *> e;
	for (Set<CharString>::Element *E = device_extensions.front(); E; E = E->next()) {
		e.push_back(E->get().ptr());
	}

	create_info.enabledExtensionCount = e.size();
	create_info.ppEnabledExtensionNames = e.ptr();

	create_info.enabledLayerCount = 0;

	if (vkCreateDevice(physical_device, &create_info, NULL, &get_device()) != VK_SUCCESS) {
		ERR_FAIL_V(ERR_CANT_CREATE, "Can't Create A Vulkan Logical Device.");
	}

	glad_vk_version = gladLoaderLoadVulkan(get_instance(), physical_device, get_device());
	if (!glad_vk_version) {
		ERR_FAIL_V(ERR_CANT_CREATE, "Can't Re-load Vulkan Symbols With Device.");
	}

	vkGetDeviceQueue(get_device(), indices.graphics_family, 0, &get_graphics_queue());
	vkGetDeviceQueue(get_device(), indices.present_family, 0, &get_present_queue());

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

	if (!is_swap_chain_adequate(p_physical_device)) {
		return -1;
	}

	if (!check_device_extension_support(p_physical_device)) {
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

#endif
