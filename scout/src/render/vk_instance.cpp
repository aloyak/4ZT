#define VOLK_IMPLEMENTATION
#define VMA_IMPLEMENTATION

#include "4ZTScout/scout.h"
#include "4ZTScout/render/vk_instance.h"
#include "4ZTScout/platform/window.h"

#include <VkBootstrap.h>

#include <cstdint>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

#ifndef NDEBUG
constexpr bool kEnableValidation = true;
#else
constexpr bool kEnableValidation = false;
#endif

void vkcheck(VkResult result) {
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Vulkan error: " + std::to_string(static_cast<int>(result)));
    }
}

// Unwraps a vk-bootstrap Result or throws with its error message.
template <typename T>
T Unwrap(vkb::Result<T>&& r, const char* what) {
    if (!r) throw std::runtime_error(std::string(what) + ": " + r.error().message());
    return std::move(r.value());
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT* data,
    void*) {
    std::fprintf(stderr, "[Vulkan %s] %s\n",
        (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) ? "ERROR" : "WARN",
        data->pMessage);
    return VK_FALSE;
}

} // namespace

Scout::VulkanInstance::VulkanInstance(Context& context) {
    try {
        vkcheck(volkInitialize());

        if (!context.window || !context.window->GetHandle())
            throw std::runtime_error("VulkanInstance requires a window");

        // instance (+ validation layer and debug messenger when available)
        uint32_t sdlExtCount = 0;
        char const* const* sdlExts = SDL_Vulkan_GetInstanceExtensions(&sdlExtCount);
        if (!sdlExts) throw std::runtime_error("SDL_Vulkan_GetInstanceExtensions failed");

        // Hand vk-bootstrap volk's loader entry point so both use the same loader.
        vkb::InstanceBuilder instanceBuilder{ vkGetInstanceProcAddr };
        instanceBuilder
            .set_app_name("4ZT Scout")
            .require_api_version(1, 3, 0)
            .enable_extensions(sdlExtCount, sdlExts);
        if (kEnableValidation) {
            instanceBuilder
                .request_validation_layers()
                .set_debug_callback(DebugCallback)
                .set_debug_messenger_severity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                              VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
                .set_debug_messenger_type(VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                          VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                          VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT);
        }
        vkb::Instance vkbInstance = Unwrap(instanceBuilder.build(), "Failed to create Vulkan instance");

        m_instance = vkbInstance.instance;
        m_debugMessenger = vkbInstance.debug_messenger;
        volkLoadInstance(m_instance);

        // surface
        if (!SDL_Vulkan_CreateSurface(context.window->GetHandle(), m_instance, nullptr, &m_surface))
            throw std::runtime_error(std::string("SDL_Vulkan_CreateSurface failed: ") + SDL_GetError());

        // physical device: Vulkan 1.3, required features, graphics + present support.
        // Prefers a discrete GPU and falls back to any other type.
        VkPhysicalDeviceFeatures features{};
        features.samplerAnisotropy = VK_TRUE;

        VkPhysicalDeviceVulkan12Features vk12{};
        vk12.descriptorIndexing = VK_TRUE;
        vk12.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
        vk12.descriptorBindingPartiallyBound = VK_TRUE;
        vk12.descriptorBindingVariableDescriptorCount = VK_TRUE;
        vk12.runtimeDescriptorArray = VK_TRUE;
        vk12.bufferDeviceAddress = VK_TRUE;

        VkPhysicalDeviceVulkan13Features vk13{};
        vk13.synchronization2 = VK_TRUE;
        vk13.dynamicRendering = VK_TRUE;

        vkb::PhysicalDeviceSelector selector{ vkbInstance };
        vkb::PhysicalDevice vkbPhysical = Unwrap(
            selector
                .set_surface(m_surface)
                .set_minimum_version(1, 3)
                .set_required_features(features)
                .set_required_features_12(vk12)
                .set_required_features_13(vk13)
                .prefer_gpu_device_type(vkb::PreferredDeviceType::discrete)
                .select(),
            "Failed to select a suitable GPU");
        m_physicalDevice = vkbPhysical.physical_device;

        // logical device (the swapchain extension is enabled automatically because a surface is set)
        vkb::Device vkbDevice = Unwrap(vkb::DeviceBuilder{ vkbPhysical }.build(),
                                       "Failed to create Vulkan device");
        m_device = vkbDevice.device;
        volkLoadDevice(m_device);

        // The renderer submits and presents on a single queue, so it must do both.
        uint32_t graphicsFamily = Unwrap(vkbDevice.get_queue_index(vkb::QueueType::graphics),
                                         "No graphics queue");
        uint32_t presentFamily  = Unwrap(vkbDevice.get_queue_index(vkb::QueueType::present),
                                         "No present queue");
        if (graphicsFamily != presentFamily)
            throw std::runtime_error("No queue family supports graphics + presentation");

        m_queueFamily = graphicsFamily;
        vkGetDeviceQueue(m_device, m_queueFamily, 0, &m_queue);

        // VMA
        VmaVulkanFunctions vkFunctions{
            .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
            .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
        };
        VmaAllocatorCreateInfo allocatorCI{
            .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
            .physicalDevice = m_physicalDevice,
            .device = m_device,
            .pVulkanFunctions = &vkFunctions,
            .instance = m_instance,
            .vulkanApiVersion = VK_API_VERSION_1_3,
        };
        vkcheck(vmaCreateAllocator(&allocatorCI, &m_allocator));
    } catch (...) {
        // A throwing constructor never runs the destructor!
        Destroy();
        throw;
    }
}

Scout::VulkanInstance::~VulkanInstance() {
    Destroy();
}

void Scout::VulkanInstance::Destroy() {
    if (m_device) vkDeviceWaitIdle(m_device);
    if (m_allocator) { vmaDestroyAllocator(m_allocator); m_allocator = VK_NULL_HANDLE; }
    if (m_device)    { vkDestroyDevice(m_device, nullptr); m_device = VK_NULL_HANDLE; }
    if (m_debugMessenger) {
        vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
        m_debugMessenger = VK_NULL_HANDLE;
    }
    if (m_surface) {
        SDL_Vulkan_DestroySurface(m_instance, m_surface, nullptr);
        m_surface = VK_NULL_HANDLE;
    }
    if (m_instance)  { vkDestroyInstance(m_instance, nullptr); m_instance = VK_NULL_HANDLE; }
}

Scout::VulkanInstance::VulkanInstance(VulkanInstance&& o) noexcept
    : m_instance(std::exchange(o.m_instance, VK_NULL_HANDLE)),
      m_debugMessenger(std::exchange(o.m_debugMessenger, VK_NULL_HANDLE)),
      m_physicalDevice(std::exchange(o.m_physicalDevice, VK_NULL_HANDLE)),
      m_device(std::exchange(o.m_device, VK_NULL_HANDLE)),
      m_queue(std::exchange(o.m_queue, VK_NULL_HANDLE)),
      m_queueFamily(std::exchange(o.m_queueFamily, UINT32_MAX)),
      m_surface(std::exchange(o.m_surface, VK_NULL_HANDLE)),
      m_allocator(std::exchange(o.m_allocator, VK_NULL_HANDLE)) {}

Scout::VulkanInstance& Scout::VulkanInstance::operator=(VulkanInstance&& o) noexcept {
    if (this != &o) {
        Destroy();
        m_instance       = std::exchange(o.m_instance, VK_NULL_HANDLE);
        m_debugMessenger = std::exchange(o.m_debugMessenger, VK_NULL_HANDLE);
        m_physicalDevice = std::exchange(o.m_physicalDevice, VK_NULL_HANDLE);
        m_device         = std::exchange(o.m_device, VK_NULL_HANDLE);
        m_queue          = std::exchange(o.m_queue, VK_NULL_HANDLE);
        m_queueFamily    = std::exchange(o.m_queueFamily, UINT32_MAX);
        m_surface        = std::exchange(o.m_surface, VK_NULL_HANDLE);
        m_allocator      = std::exchange(o.m_allocator, VK_NULL_HANDLE);
    }
    return *this;
}