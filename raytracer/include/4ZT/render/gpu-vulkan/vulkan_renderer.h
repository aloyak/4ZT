#pragma once

#include "4ZT/render/gpu-vulkan/vulkan_deps.h"

#include "4ZT/render/render.h"

#include "4ZT/settings.h"
#include "4ZT/camera.h"
#include "4ZT/world/world.h"
#include "4ZT/output/image.h"

namespace FourZT {

class VulkanRenderer : public Renderer {
public:
    VulkanRenderer(const Settings& settings);
    ~VulkanRenderer() override { Destroy(); }

    Image Render(const Settings& settings,
                 const Camera& camera,
                 const World& world) override;

private:
    void Initialize();
    void Destroy();

    VkInstance               m_instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice         m_physicalDevice = VK_NULL_HANDLE;
    VkDevice                 m_device = VK_NULL_HANDLE;
    VkQueue                  m_queue = VK_NULL_HANDLE;
    uint32_t                 m_queueFamily = UINT32_MAX;
    VmaAllocator             m_allocator = VK_NULL_HANDLE;
    VkCommandPool            m_commandPool = VK_NULL_HANDLE;

    int m_imgHeight = 0;
    int m_imgWidth = 0;
};

}