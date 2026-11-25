#include "4ZTScout/render/swapchain.h"

#include <VkBootstrap.h>

#include <stdexcept>
#include <string>

namespace {
void vkcheck(VkResult r) {
    if (r != VK_SUCCESS) throw std::runtime_error("Vulkan error: " + std::to_string(static_cast<int>(r)));
}
}

bool Scout::Swapchain::Create(VulkanInstance& vk, SDL_Window* window) {
    m_vk = &vk;
    m_window = window;
    return Build();
}

bool Scout::Swapchain::Recreate() {
    vkDeviceWaitIdle(m_vk->GetDevice());
    return Build();
}

bool Scout::Swapchain::NeedsRecreate() const {
    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(m_window, &w, &h);
    return w > 0 && h > 0 &&
           (static_cast<uint32_t>(w) != m_extent.width || static_cast<uint32_t>(h) != m_extent.height);
}

bool Scout::Swapchain::Build() {
    VkDevice device = m_vk->GetDevice();

    int w = 0, h = 0;
    SDL_GetWindowSizeInPixels(m_window, &w, &h);
    if (w <= 0 || h <= 0) return false; // minimized

    VkSwapchainKHR old = m_swapchain;

    // vk-bootstrap handles surface capabilities, extent clamping, image count (min + 1,
    // clamped to max) and format/present-mode fallback. FIFO is always available (vsync).
    vkb::SwapchainBuilder builder{ m_vk->GetPhysicalDevice(), device, m_vk->GetSurface() };
    auto result = builder
        .set_desired_format({ VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
        .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
        .set_desired_extent(static_cast<uint32_t>(w), static_cast<uint32_t>(h))
        .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT) // color attachment is the default
        .set_clipped(true)
        .set_old_swapchain(old)
        .build();
    if (!result)
        throw std::runtime_error("Failed to create swapchain: " + result.error().message());
    vkb::Swapchain vkbSwapchain = result.value();

    // old per-image objects are safe to destroy (caller waited for idle)
    DestroyPerImage();
    if (old) vkDestroySwapchainKHR(device, old, nullptr);

    m_swapchain = vkbSwapchain.swapchain;
    m_format = vkbSwapchain.image_format;
    m_extent = vkbSwapchain.extent;

    auto images = vkbSwapchain.get_images();
    auto views = vkbSwapchain.get_image_views();
    if (!images || !views)
        throw std::runtime_error("Failed to get swapchain images/views");
    m_images = std::move(images.value());
    m_views = std::move(views.value());

    // One render-finished semaphore per swapchain image, so present never waits on one still in use.
    m_renderFinished.resize(m_images.size());
    for (auto& s : m_renderFinished) {
        VkSemaphoreCreateInfo si{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        vkcheck(vkCreateSemaphore(device, &si, nullptr, &s));
    }
    return true;
}

void Scout::Swapchain::DestroyPerImage() {
    VkDevice device = m_vk->GetDevice();
    for (auto v : m_views) vkDestroyImageView(device, v, nullptr);
    for (auto s : m_renderFinished) vkDestroySemaphore(device, s, nullptr);
    m_views.clear();
    m_renderFinished.clear();
    m_images.clear();
}

void Scout::Swapchain::Destroy() {
    if (!m_vk || !m_vk->GetDevice()) return;
    vkDeviceWaitIdle(m_vk->GetDevice());
    DestroyPerImage();
    if (m_swapchain) {
        vkDestroySwapchainKHR(m_vk->GetDevice(), m_swapchain, nullptr);
        m_swapchain = VK_NULL_HANDLE;
    }
}