#pragma once

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1

#include <volk.h>
#include <vk_mem_alloc.h>

namespace Scout {

struct Context;

class VulkanInstance {
public:
    VulkanInstance() = default;
    explicit VulkanInstance(Context& context);
    ~VulkanInstance();

    VulkanInstance(const VulkanInstance&) = delete;
    VulkanInstance& operator=(const VulkanInstance&) = delete;
    VulkanInstance(VulkanInstance&& other) noexcept;
    VulkanInstance& operator=(VulkanInstance&& other) noexcept;

    VkInstance       GetInstance() const       { return m_instance; }
    VkPhysicalDevice GetPhysicalDevice() const { return m_physicalDevice; }
    VkDevice         GetDevice() const         { return m_device; }
    VkQueue          GetQueue() const          { return m_queue; }
    uint32_t         GetQueueFamily() const    { return m_queueFamily; }
    VkSurfaceKHR     GetSurface() const        { return m_surface; }
    VmaAllocator     GetAllocator() const      { return m_allocator; }

private:
    void Destroy();

    VkInstance               m_instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice         m_physicalDevice = VK_NULL_HANDLE;
    VkDevice                 m_device = VK_NULL_HANDLE;
    VkQueue                  m_queue = VK_NULL_HANDLE;
    uint32_t                 m_queueFamily = UINT32_MAX;
    VkSurfaceKHR             m_surface = VK_NULL_HANDLE;
    VmaAllocator             m_allocator = VK_NULL_HANDLE;
};

} // scout