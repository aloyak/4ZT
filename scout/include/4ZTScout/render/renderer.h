#pragma once

#include "4ZTScout/layer.h"
#include "4ZTScout/render/vk_instance.h"
#include "4ZTScout/render/swapchain.h"

#include <array>

namespace Scout {

class Window;
class GUI;
class VulkanInstance;
class Swapchain;

class Renderer : public Layer {
public:
    void OnAttach(Context& context) override;
    void OnDetach(Context& context) override;
    void OnUpdate(float dt, Context& context) override {}
    void OnRender(Context& context) override;

    VulkanInstance& GetVulkanInstance() { return m_vkInstance; }
    Swapchain& GetSwapchain() { return m_swapchain; }

private:
    static constexpr uint32_t kFramesInFlight = 2;

    struct Frame {
        VkCommandPool   pool = VK_NULL_HANDLE;
        VkCommandBuffer cmd = VK_NULL_HANDLE;
        VkSemaphore     imageAvailable = VK_NULL_HANDLE;
        VkFence         inFlight = VK_NULL_HANDLE;
    };

    void TransitionImage(VkCommandBuffer cmd, VkImage image,
                         VkImageLayout from, VkImageLayout to,
                         VkPipelineStageFlags2 srcStage, VkAccessFlags2 srcAccess,
                         VkPipelineStageFlags2 dstStage, VkAccessFlags2 dstAccess);

    VulkanInstance m_vkInstance;
    Swapchain m_swapchain;
    std::array<Frame, kFramesInFlight> m_frames{};
    uint32_t m_frameIndex = 0;
    Window* m_window = nullptr;
};

} // scout