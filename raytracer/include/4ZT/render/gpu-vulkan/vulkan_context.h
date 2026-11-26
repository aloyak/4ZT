#pragma once

#include "4ZT/render/gpu-vulkan/vulkan_deps.h"

#include "4ZT/settings.h"

namespace FourZT {

class VulkanContext {
public:
    VulkanContext() = default;
    ~VulkanContext() { Destroy(); }
    VulkanContext(const VulkanContext&) = delete;
    VulkanContext& operator=(const VulkanContext&) = delete;

    void Initialize(const Settings& settings);
    void Destroy();

    VkInstance       GetInstance() const       { return m_instance; }
    VkPhysicalDevice GetPhysicalDevice() const { return m_physicalDevice; }
    VkDevice         GetDevice() const         { return m_device; }
    VkQueue          GetQueue() const          { return m_queue; }
    uint32_t         GetQueueFamily() const    { return m_queueFamily; }
    VmaAllocator     GetAllocator() const      { return m_allocator; }
    VkCommandPool    GetCommandPool() const    { return m_commandPool; }

private:
    VkInstance               m_instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice         m_physicalDevice = VK_NULL_HANDLE;
    VkDevice                 m_device = VK_NULL_HANDLE;
    VkQueue                  m_queue = VK_NULL_HANDLE;
    uint32_t                 m_queueFamily = UINT32_MAX;
    VmaAllocator             m_allocator = VK_NULL_HANDLE;
    VkCommandPool            m_commandPool = VK_NULL_HANDLE;
};

} // 4zt