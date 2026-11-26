#pragma once

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1

#include <volk.h>
#include <vk_mem_alloc.h>

#include <stdexcept>

namespace FourZT {

// lil helper
inline void vkcheck(VkResult result) {
    if (result != VK_SUCCESS)
        throw std::runtime_error("4ZT Vulkan error: " + std::to_string(static_cast<int>(result)));
}

} // 4zt