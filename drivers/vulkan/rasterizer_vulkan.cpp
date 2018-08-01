#include "drivers/vulkan/rasterizer_vulkan.h"

#include "platform/windows/os_windows.h"
#include "platform/windows/rendering_context_vulkan_win.h"

Rasterizer *MakeCurrentFunctVulkan::make_current() {
	return memnew(RasterizerVulkan(context));
}

MakeCurrentFunctVulkan::MakeCurrentFunctVulkan(RenderingContext *p_context) {
	context = p_context;
}

MakeCurrentFunctVulkan::~MakeCurrentFunctVulkan() {
}

RenderingContextVulkan_Win *RasterizerVulkan::get_instance_vulkan() {
	return dynamic_cast<RenderingContextVulkan_Win *>(context);
}

void RasterizerVulkan::initialize() {
	if (OS::get_singleton()->is_stdout_verbose()) {
		print_line("Using Vulkan video driver");
	}

	get_instance_vulkan()->create_image_views();
	get_instance_vulkan()->create_render_pass();
	get_instance_vulkan()->create_graphics_pipeline();
	get_instance_vulkan()->create_framebuffers();
	get_instance_vulkan()->create_command_pool();
	get_instance_vulkan()->create_command_buffers();
	get_instance_vulkan()->create_semaphores();
}

void RasterizerVulkan::end_frame(bool p_swap_buffers) {
	if (p_swap_buffers == false) {
		return;
	}
	uint32_t image_index;
	vkAcquireNextImageKHR(get_instance_vulkan()->get_device(), get_instance_vulkan()->get_swap_chain(), UINT64_MAX, get_instance_vulkan()->get_image_available_semaphore(), VK_NULL_HANDLE, &image_index);
	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore wait_semaphores[] = { get_instance_vulkan()->get_image_available_semaphore() };
	VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;

	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &get_instance_vulkan()->get_command_buffers()[image_index];

	VkSemaphore signal_semaphores[] = { get_instance_vulkan()->get_render_finished_semaphore() };
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signal_semaphores;

	if (vkQueueSubmit(get_instance_vulkan()->get_graphics_queue(), 1, &submit_info, VK_NULL_HANDLE) != VK_SUCCESS) {
		ERR_FAIL("Can't Submit Draw Command Buffer.");
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signal_semaphores;

	VkSwapchainKHR swap_chains[] = { get_instance_vulkan()->get_swap_chain() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swap_chains;

	presentInfo.pImageIndices = &image_index;

	vkQueuePresentKHR(get_instance_vulkan()->get_present_queue(), &presentInfo);
}

void RasterizerVulkan::make_current(RenderingContext *context) {
	_create_func = memnew(MakeCurrentFunctVulkan(context));
}

void RasterizerVulkan::register_config() {
}
