#include "4ZTScout/render/swapchain.h"

#include <algorithm>
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
    VkPhysicalDevice pd = m_vk->GetPhysicalDevice();
    VkDevice device = m_vk->GetDevice();
    VkSurfaceKHR surface = m_vk->GetSurface();

    VkSurfaceCapabilitiesKHR caps;
    vkcheck(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pd, surface, &caps));

    VkExtent2D extent = caps.currentExtent;
    if (extent.width == UINT32_MAX) {
        int w = 0, h = 0;
        SDL_GetWindowSizeInPixels(m_window, &w, &h);
        extent.width  = std::clamp<uint32_t>(w, caps.minImageExtent.width,  caps.maxImageExtent.width);
        extent.height = std::clamp<uint32_t>(h, caps.minImageExtent.height, caps.maxImageExtent.height);
    }
    if (extent.width == 0 || extent.height == 0) return false; // minimized

    // format
    uint32_t fmtCount = 0;
    vkcheck(vkGetPhysicalDeviceSurfaceFormatsKHR(pd, surface, &fmtCount, nullptr));
    std::vector<VkSurfaceFormatKHR> formats(fmtCount);
    vkcheck(vkGetPhysicalDeviceSurfaceFormatsKHR(pd, surface, &fmtCount, formats.data()));
    VkSurfaceFormatKHR chosen = formats[0];
    for (auto& f : formats) {
        if (f.format == VK_FORMAT_B8G8R8A8_SRGB && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            chosen = f;
            break;
        }
    }

    uint32_t imageCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0) imageCount = std::min(imageCount, caps.maxImageCount);

    VkSwapchainKHR old = m_swapchain;

    VkSwapchainCreateInfoKHR ci{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = imageCount,
        .imageFormat = chosen.format,
        .imageColorSpace = chosen.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = caps.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR, // always available, vsynced
        .clipped = VK_TRUE,
        .oldSwapchain = old,
    };

    VkSwapchainKHR fresh = VK_NULL_HANDLE;
    vkcheck(vkCreateSwapchainKHR(device, &ci, nullptr, &fresh));

    // old per-image objects are safe to destroy (caller waited for idle)
    DestroyPerImage();
    if (old) vkDestroySwapchainKHR(device, old, nullptr);

    m_swapchain = fresh;
    m_format = chosen.format;
    m_extent = extent;

    uint32_t count = 0;
    vkcheck(vkGetSwapchainImagesKHR(device, m_swapchain, &count, nullptr));
    m_images.resize(count);
    vkcheck(vkGetSwapchainImagesKHR(device, m_swapchain, &count, m_images.data()));

    m_views.resize(count);
    m_renderFinished.resize(count);
    for (uint32_t i = 0; i < count; ++i) {
        VkImageViewCreateInfo vi{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = m_images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = m_format,
            .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
        };
        vkcheck(vkCreateImageView(device, &vi, nullptr, &m_views[i]));

        VkSemaphoreCreateInfo si{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        vkcheck(vkCreateSemaphore(device, &si, nullptr, &m_renderFinished[i]));
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