/*************************************************************************/
/*  vma_usage.h				                                             */
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
#ifndef VULKAN_MEMORY_ALLOCATOR_H
#define VULKAN_MEMORY_ALLOCATOR_H

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VkMemoryDedicatedAllocateInfoKHR VkMemoryDedicatedAllocateInfo
#define VkBufferMemoryRequirementsInfo2KHR VkBufferMemoryRequirementsInfo2
#define VkMemoryDedicatedRequirementsKHR VkMemoryDedicatedRequirements
#define VkMemoryRequirements2KHR VkMemoryRequirements2
#define VkImageMemoryRequirementsInfo2KHR VkImageMemoryRequirementsInfo2

#include "reference.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#define NOMINMAX
#define VMA_MAX MAX
#define VMA_MIN MIN
#pragma warning(push, 4)
#pragma warning(disable : 4127) // conditional expression is constant
#pragma warning(disable : 4100) // unreferenced formal parameter
#pragma warning(disable : 4189) // local variable is initialized but not referenced
#endif

#include "glad/vulkan.h"

#include "thirdparty/vulkan_memory_allocator/vk_mem_alloc.h"

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
