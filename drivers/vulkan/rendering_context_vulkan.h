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

#if defined(VULKAN_ENABLED)

#include "core/math/camera_matrix.h"
#include "core/set.h"
#include "core/ustring.h"
#include "core/vector.h"
#include "servers/visual/rendering_context.h"
#include "typedefs.h"

#include "glad/vulkan.h"

class RenderingContextVulkan : public RenderingContext {
private:
	const bool enable_validation = true;

	const int WIDTH = 800;
	const int HEIGHT = 600;

	unsigned int pixel_format;
	bool use_vsync;
	int glad_vk_version = 0;

private:
	struct Vertex {
		Vector2 pos;
		Color color;
		Vector2 texCoord;

		static VkVertexInputBindingDescription get_binding_description() {
			VkVertexInputBindingDescription bindingDescription = {};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return bindingDescription;
		}

		static Vector<VkVertexInputAttributeDescription> get_attribute_descriptions() {
			Vector<VkVertexInputAttributeDescription> attribute_descriptions;
			attribute_descriptions.resize(3);
			attribute_descriptions.write[0].binding = 0;
			attribute_descriptions.write[0].location = 0;
			attribute_descriptions.write[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attribute_descriptions.write[0].offset = offsetof(Vertex, pos);

			attribute_descriptions.write[1].binding = 0;
			attribute_descriptions.write[1].location = 1;
			attribute_descriptions.write[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attribute_descriptions.write[1].offset = offsetof(Vertex, color);

			attribute_descriptions.write[2].binding = 0;
			attribute_descriptions.write[2].location = 2;
			attribute_descriptions.write[2].format = VK_FORMAT_R32G32_SFLOAT;
			attribute_descriptions.write[2].offset = offsetof(Vertex, texCoord);

			return attribute_descriptions;
		}

		bool operator==(const Vertex &other) const {
			return pos == other.pos && color == other.color && texCoord == other.texCoord;
		}
	};

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

	struct UniformBufferObject {
		CameraMatrix model;
		CameraMatrix view;
		CameraMatrix proj;
	};

	VkPhysicalDevice physical_device = VK_NULL_HANDLE;
	VkDevice device = VK_NULL_HANDLE;

	VkSwapchainKHR swap_chain;
	Vector<VkImage> swap_chain_images;
	VkFormat swap_chain_image_format;
	VkExtent2D swap_chain_extent;
	Vector<VkImageView> swap_chain_image_views;
	Vector<VkFramebuffer> swap_chain_framebuffers;

	VkQueue graphics_queue;
	VkQueue present_queue;

	VkRenderPass render_pass;

	VkDescriptorSetLayout descriptor_set_layout;

	VkPipelineLayout pipeline_layout;
	VkPipeline graphics_pipeline;

	// Buffers
	Vector<Vertex> vertices;
	Vector<uint32_t> indices;

	VkBuffer vertex_buffer;
	VkDeviceMemory vertex_buffer_memory;
	VkBuffer index_buffer;
	VkDeviceMemory index_buffer_memory;
	Vector<VkBuffer> uniform_buffers;
	Vector<VkDeviceMemory> uniform_buffers_memory;
	VkDescriptorPool descriptor_pool;
	Vector<VkDescriptorSet> descriptor_sets;

	// Texture
	uint32_t mip_levels;
	VkImage texture_image;
	VkDeviceMemory texture_image_memory;
	VkImageView texture_image_view;
	VkSampler texture_sampler;

	VkImage depth_image;
	VkDeviceMemory depth_image_memory;
	VkImageView depth_image_view;

	VkCommandPool command_pool;
	Vector<VkCommandBuffer> command_buffers;

	// Synchronisation
	const int MAX_FRAMES_IN_FLIGHT = 2;
	Vector<VkSemaphore> image_available_semaphores;
	Vector<VkSemaphore> render_finished_semaphores;
	Vector<VkFence> in_flight_fences;
	size_t currentFrame = 0;
	bool framebuffer_resized = false;

	// Instance creation
	Error _create_instance();
	Vector<const char *> validation_layers;

	// Validation layers
	bool _check_validation_layer_support(Vector<const char *> validationLayers);
	VkResult _create_debug_report_callback_EXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugReportCallbackEXT *pCallback);
	void _destroy_debug_report_callback_EXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks *pAllocator);
	static VKAPI_ATTR VkBool32 VKAPI_CALL _debug_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType, uint64_t obj, size_t location, int32_t code, const char *layerPrefix, const char *msg, void *userData);
	void _enable_debug();

	// Create surface -> protected

	// Physical device selection
	struct RenderingContextVulkan::QueueFamilyIndices _pick_queue_families(VkPhysicalDevice p_physical_device);
	bool _check_device_extension_support(VkPhysicalDevice device, Set<CharString> device_extensions);
	int _get_physical_device_score(const VkPhysicalDevice p_physical_device);
	Error _pick_physical_device();

	// Create logical device
	Error _create_logical_device();

	// Create swap chain
	RenderingContextVulkan::SwapChainSupportDetails _query_swap_chain_support(VkPhysicalDevice device);
	VkSurfaceFormatKHR _choose_swap_surface_format(const Vector<VkSurfaceFormatKHR> &available_formats);
	VkPresentModeKHR _choose_swap_present_mode(const Vector<VkPresentModeKHR> available_present_modes);
	VkExtent2D _choose_swap_extent(const VkSurfaceCapabilitiesKHR &capabilities);
	bool _is_swap_chain_adequate(const VkPhysicalDevice p_physical_device);
	Error _create_swap_chain();
	void _cleanup_swap_chain();
	void _recreate_swap_chain();

	// Create image views
	Error _create_image_views();

	// Create the render pass
	void _create_render_pass();

	// Create graphic pipeline
	VkShaderModule _create_shader_module(Vector<uint8_t> code);
	void _create_descriptor_set_layout();
	void _create_graphic_pipeline();

	// Helpers
	VkCommandBuffer _begin_single_time_commands();
	void _end_single_time_commands(VkCommandBuffer command_buffer);
	VkImageView _create_image_view(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mip_levels);

	// Buffers operations
	uint32_t _find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void _create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &buffer_memory);
	void _copy_buffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void _copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void _transition_image_layout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mip_levels);
	void _create_image(uint32_t width, uint32_t height, uint32_t mip_levels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);
	void _create_mipmaps(VkImage image, VkFormat format, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

	void _create_texture_image();
	void _create_texture_image_view();
	void _create_texture_sampler();
	//int _load_model();
	void _load_simple_quad();
	int _create_vertex_buffer();
	int _create_index_buffer();
	int _create_uniform_buffers();
	void _create_descriptor_pool();
	void _create_descriptor_sets();

	// Depth buffer
	VkFormat _find_supported_format(const Vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	VkFormat _find_depth_format();
	bool _has_stencil_component(VkFormat format);
	int _create_depth_resources();

	// Commands
	void _create_framebuffers();
	void _create_command_buffers();
	void _create_command_pool();

	// Drawing
	void _update_uniform_buffer(uint32_t currentImage);

	// Create the semaphores and fences
	Error _create_semaphores_and_fences();

	// Cleanup
	void cleanup();

protected:
	VkInstance instance;

	// Get surface extension
	virtual char *_get_surface_extension() const = 0;

	// Window creation
	virtual Error _create_window() = 0;

	// Surface creation
	VkSurfaceKHR surface;
	virtual void _create_surface() = 0;

public:
	static VkDebugReportCallbackEXT callback;

	virtual void release_current();

	virtual void make_current();

	virtual void swap_buffers();

	virtual Error initialize();

	virtual void set_use_vsync(bool p_use);
	virtual bool is_using_vsync() const;

	~RenderingContextVulkan();
};

#endif
#endif
