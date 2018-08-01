#include "rasterizer_vulkan.h"

Rasterizer *MakeCurrentFunctVulkan::make_current() {
	return memnew(RasterizerVulkan(context));
}

MakeCurrentFunctVulkan::MakeCurrentFunctVulkan(RenderingContext *p_context) {
	context = p_context;
}

MakeCurrentFunctVulkan::~MakeCurrentFunctVulkan() {
}

void RasterizerVulkan::make_current(RenderingContext *context) {
	_create_func = memnew(MakeCurrentFunctVulkan(context));
}

void RasterizerVulkan::register_config() {
}
