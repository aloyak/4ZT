#pragma once

#include <SDL3/SDL.h>

#include "4ZTScout/render/vk_instance.h"
#include <vector>

namespace Scout {

class Swapchain {
public:
    Swapchain() = default;
    ~Swapchain() { Destroy(); }
    Swapchain(const Swapchain&) = delete;
    Swapchain& operator=(const Swapchain&) = delete;

    // Returns false if the window has zero size (minimized).
    bool Create(VulkanInstance& vk, SDL_Window* window);
    bool Recreate();   // waits for idle, keeps the old swapchain as oldSwapchain
    void Destroy();

    VkSwapchainKHR Handle() const { return m_swapchain; }
    VkFormat Format() const { return m_format; }
    VkExtent2D Extent() const { return m_extent; }
    VkImage Image(uint32_t i) const { return m_images[i]; }
    VkImageView View(uint32_t i) const { return m_views[i]; }
    // One per swapchain image, so present never waits on a semaphore still in use.
    VkSemaphore RenderFinished(uint32_t i) const { return m_renderFinished[i]; }
    bool IsValid() const { return m_swapchain != VK_NULL_HANDLE; }

    // True if the window's pixel size no longer matches the swapchain.
    bool NeedsRecreate() const;

private:
    bool Build();
    void DestroyPerImage();

    VulkanInstance* m_vk = nullptr;
    SDL_Window* m_window = nullptr;

    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    VkFormat m_format = VK_FORMAT_UNDEFINED;
    VkExtent2D m_extent{};
    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_views;
    std::vector<VkSemaphore> m_renderFinished;
};

} // scout