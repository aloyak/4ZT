#include "4ZTScout/render/renderer.h"
#include "4ZTScout/scout.h"
#include "4ZTScout/platform/window.h"

#include <cstdio>
#include <stdexcept>

void Scout::Renderer::OnAttach(Context& context) {
    try {
        m_window = context.window;
        m_vkInstance = VulkanInstance(context);
        m_swapchain.Create(m_vkInstance, m_window->GetHandle());

        VkDevice device = m_vkInstance.GetDevice();
        for (auto& f : m_frames) {
            VkCommandPoolCreateInfo pci{
                .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .queueFamilyIndex = m_vkInstance.GetQueueFamily(),
            };
            if (vkCreateCommandPool(device, &pci, nullptr, &f.pool) != VK_SUCCESS)
                throw std::runtime_error("vkCreateCommandPool failed");

            VkCommandBufferAllocateInfo ai{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool = f.pool,
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1,
            };
            if (vkAllocateCommandBuffers(device, &ai, &f.cmd) != VK_SUCCESS)
                throw std::runtime_error("vkAllocateCommandBuffers failed");

            VkSemaphoreCreateInfo si{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
            vkCreateSemaphore(device, &si, nullptr, &f.imageAvailable);

            VkFenceCreateInfo fi{
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .flags = VK_FENCE_CREATE_SIGNALED_BIT,
            };
            vkCreateFence(device, &fi, nullptr, &f.inFlight);
        }
    } catch (const std::exception& e) {
        std::fprintf(stderr, "Renderer init failed: %s\n", e.what());
        if (context.running) *context.running = false;
    }
}

void Scout::Renderer::OnDetach(Context&) {
    // Must tear down here: Platform::OnDetach (runs after us) destroys the window and quits SDL.
    VkDevice device = m_vkInstance.GetDevice();
    if (device) {
        vkDeviceWaitIdle(device);
        for (auto& f : m_frames) {
            if (f.inFlight)       vkDestroyFence(device, f.inFlight, nullptr);
            if (f.imageAvailable) vkDestroySemaphore(device, f.imageAvailable, nullptr);
            if (f.pool)           vkDestroyCommandPool(device, f.pool, nullptr); // frees its cmd buffer
            f = {};
        }
    }
    m_swapchain.Destroy();
    m_vkInstance = VulkanInstance{};   // move-assign: destroys device/surface/instance now
    m_window = nullptr;
}

void Scout::Renderer::TransitionImage(VkCommandBuffer cmd, VkImage image,
                                      VkImageLayout from, VkImageLayout to,
                                      VkPipelineStageFlags2 srcStage, VkAccessFlags2 srcAccess,
                                      VkPipelineStageFlags2 dstStage, VkAccessFlags2 dstAccess) {
    VkImageMemoryBarrier2 barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = srcStage,
        .srcAccessMask = srcAccess,
        .dstStageMask = dstStage,
        .dstAccessMask = dstAccess,
        .oldLayout = from,
        .newLayout = to,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
    };
    VkDependencyInfo dep{
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier,
    };
    vkCmdPipelineBarrier2(cmd, &dep);
}

void Scout::Renderer::OnRender(Context&) {
    if (!m_vkInstance.GetDevice() || !m_window) return;

    VkDevice device = m_vkInstance.GetDevice();

    // Resize / minimize handling
    if (!m_swapchain.IsValid() || m_swapchain.NeedsRecreate()) {
        if (!m_swapchain.Recreate()) return;   // minimized, skip the frame
    }

    Frame& frame = m_frames[m_frameIndex];

    vkWaitForFences(device, 1, &frame.inFlight, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex = 0;
    VkResult acq = vkAcquireNextImageKHR(device, m_swapchain.Handle(), UINT64_MAX,
                                         frame.imageAvailable, VK_NULL_HANDLE, &imageIndex);
    if (acq == VK_ERROR_OUT_OF_DATE_KHR) {
        m_swapchain.Recreate();
        return;
    }
    if (acq != VK_SUCCESS && acq != VK_SUBOPTIMAL_KHR) {
        std::fprintf(stderr, "vkAcquireNextImageKHR failed: %d\n", acq);
        return;
    }

    // Only reset the fence once we know we'll submit work that signals it.
    vkResetFences(device, 1, &frame.inFlight);
    vkResetCommandPool(device, frame.pool, 0);

    VkCommandBuffer cmd = frame.cmd;
    VkCommandBufferBeginInfo bi{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(cmd, &bi);

    VkImage image = m_swapchain.Image(imageIndex);

    TransitionImage(cmd, image,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, 0,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT);

    VkRenderingAttachmentInfo color{
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = m_swapchain.View(imageIndex),
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = { .color = { { 0.02f, 0.03f, 0.05f, 1.0f } } },
    };
    VkRenderingInfo ri{
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = { {0, 0}, m_swapchain.Extent() },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color,
    };
    vkCmdBeginRendering(cmd, &ri);
    // TODO: previewer draw calls, GUI draw data
    vkCmdEndRendering(cmd);

    TransitionImage(cmd, image,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        VK_PIPELINE_STAGE_2_NONE, 0);

    vkEndCommandBuffer(cmd);

    VkSemaphoreSubmitInfo waitInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = frame.imageAvailable,
        .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
    };
    VkSemaphoreSubmitInfo signalInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = m_swapchain.RenderFinished(imageIndex),
        .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
    };
    VkCommandBufferSubmitInfo cmdInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = cmd,
    };
    VkSubmitInfo2 submit{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .waitSemaphoreInfoCount = 1,
        .pWaitSemaphoreInfos = &waitInfo,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &cmdInfo,
        .signalSemaphoreInfoCount = 1,
        .pSignalSemaphoreInfos = &signalInfo,
    };
    vkQueueSubmit2(m_vkInstance.GetQueue(), 1, &submit, frame.inFlight);

    VkSwapchainKHR sc = m_swapchain.Handle();
    VkSemaphore renderFinished = m_swapchain.RenderFinished(imageIndex);
    VkPresentInfoKHR pi{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &renderFinished,
        .swapchainCount = 1,
        .pSwapchains = &sc,
        .pImageIndices = &imageIndex,
    };
    VkResult pres = vkQueuePresentKHR(m_vkInstance.GetQueue(), &pi);
    if (pres == VK_ERROR_OUT_OF_DATE_KHR || pres == VK_SUBOPTIMAL_KHR) {
        m_swapchain.Recreate();
    }

    m_frameIndex = (m_frameIndex + 1) % kFramesInFlight;
}