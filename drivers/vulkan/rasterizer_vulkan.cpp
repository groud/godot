#include "drivers/vulkan/rasterizer_vulkan.h"

void RasterizerVulkan::initialize() {
}

void RasterizerVulkan::end_frame(bool p_swap_buffers) {
	OS::get_singleton()->swap_buffers();
}

void RasterizerVulkan::make_current() {
	if (_instance != NULL) {
		memdelete(_instance);
	}
	_instance = memnew(RasterizerVulkan);
}

void RasterizerVulkan::register_config() {
}

RasterizerVulkan::~RasterizerVulkan() {
}
