/*************************************************************************/
/*  rendering_context_vulkan.h                                           */
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

#ifndef RENDERING_CONTEXT_VULKAN_X11_H
#define RENDERING_CONTEXT_VULKAN_X11_H

#if defined(VULKAN_ENABLED)

#include <X11/Xlib.h>
#define VK_USE_PLATFORM_XLIB_KHR
#include "glad/vulkan.h"
#undef CursorShape

#include "os/os.h"
#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>

#include "drivers/vulkan/rendering_context_vulkan.h"
#include "typedefs.h"

class RenderingContextVulkan_X11 : public RenderingContextVulkan {
private:
	OS::VideoMode default_video_mode;
	::Display *x11_display;
	::Window &x11_window;

protected:
	virtual char *_get_surface_extension() const;
	virtual Error _create_window();
	virtual void _create_surface();

public:
	virtual void set_use_vsync(bool p_use);
	virtual bool is_using_vsync() const;

	RenderingContextVulkan_X11(::Display *p_x11_display, ::Window &p_x11_window, const OS::VideoMode &p_default_video_mode);
	~RenderingContextVulkan_X11();
};

#endif

#endif
