#define VOLK_IMPLEMENTATION
#define VMA_IMPLEMENTATION

#include "4ZTScout/scout.h"
#include "4ZTScout/render/vk_instance.h"
#include "4ZTScout/platform/window.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <utility>
#include <vector>
#include <string>

namespace {

void vkcheck(VkResult result) {
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Vulkan error: " + std::to_string(static_cast<int>(result)));
    }
}

#ifndef NDEBUG
constexpr bool kEnableValidation = true;
#else
constexpr bool kEnableValidation = false;
#endif

constexpr const char* kValidationLayer = "VK_LAYER_KHRONOS_validation";

bool HasValidationLayer() {
    uint32_t count = 0;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    std::vector<VkLayerProperties> layers(count);
    vkEnumerateInstanceLayerProperties(&count, layers.data());
    for (const auto& l : layers) {
        if (std::strcmp(l.layerName, kValidationLayer) == 0) return true;
    }
    return false;
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
    vkcheck(volkInitialize());

    // instance
    VkApplicationInfo appInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "4ZT Scout",
        .apiVersion = VK_API_VERSION_1_3,
    };

    uint32_t sdlExtCount = 0;
    char const* const* sdlExts = SDL_Vulkan_GetInstanceExtensions(&sdlExtCount);
    if (!sdlExts) throw std::runtime_error("SDL_Vulkan_GetInstanceExtensions failed");

    std::vector<const char*> instanceExtensions(sdlExts, sdlExts + sdlExtCount);

    std::vector<const char*> layers;
    bool validation = kEnableValidation && HasValidationLayer();
    if (validation) {
        layers.push_back(kValidationLayer);
        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    VkDebugUtilsMessengerCreateInfoEXT messengerCI{
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = DebugCallback,
    };

    VkInstanceCreateInfo instanceCI{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = validation ? &messengerCI : nullptr, // also covers create/destroy of the instance
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(layers.size()),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size()),
        .ppEnabledExtensionNames = instanceExtensions.data(),
    };
    vkcheck(vkCreateInstance(&instanceCI, nullptr, &m_instance));
    volkLoadInstance(m_instance);

    if (!context.window || !context.window->GetHandle())
        throw std::runtime_error("VulkanInstance requires a window");

    if (!SDL_Vulkan_CreateSurface(context.window->GetHandle(), m_instance, nullptr, &m_surface))
        throw std::runtime_error(std::string("SDL_Vulkan_CreateSurface failed: ") + SDL_GetError());

    if (validation) {
        vkcheck(vkCreateDebugUtilsMessengerEXT(m_instance, &messengerCI, nullptr, &m_debugMessenger));
    }

    // physical device
    uint32_t gpuCount = 0;
    vkcheck(vkEnumeratePhysicalDevices(m_instance, &gpuCount, nullptr));
    if (gpuCount == 0) throw std::runtime_error("No Vulkan-capable GPU found");
    std::vector<VkPhysicalDevice> gpus(gpuCount);
    vkcheck(vkEnumeratePhysicalDevices(m_instance, &gpuCount, gpus.data()));

    // Prefer a discrete GPU, otherwise fall back to the first device.
    m_physicalDevice = gpus[0];
    for (VkPhysicalDevice gpu : gpus) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(gpu, &props);
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            m_physicalDevice = gpu;
            break;
        }
    }

    // queue family
    uint32_t qCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &qCount, nullptr);
    std::vector<VkQueueFamilyProperties> qFamilies(qCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &qCount, qFamilies.data());

    for (uint32_t i = 0; i < qCount; ++i) {
        bool graphics = qFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;
        bool present  = SDL_Vulkan_GetPresentationSupport(m_instance, m_physicalDevice, i);
        if (graphics && present) {
            m_queueFamily = i;
            break;
        }
    }
    if (m_queueFamily == UINT32_MAX) {
        throw std::runtime_error("No queue family supports graphics + presentation");
    }

    const float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCI{
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = m_queueFamily,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority,
    };

    // logical device
    VkPhysicalDeviceVulkan12Features vk12{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .descriptorIndexing = VK_TRUE,
        .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
        .descriptorBindingPartiallyBound = VK_TRUE,
        .descriptorBindingVariableDescriptorCount = VK_TRUE,
        .runtimeDescriptorArray = VK_TRUE,
        .bufferDeviceAddress = VK_TRUE,
    };
    VkPhysicalDeviceVulkan13Features vk13{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &vk12,
        .synchronization2 = VK_TRUE,
        .dynamicRendering = VK_TRUE,
    };
    VkPhysicalDeviceFeatures2 features2{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &vk13,
        .features = { .samplerAnisotropy = VK_TRUE },
    };

    const char* deviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo deviceCI{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &features2,                 // features go through pNext, so pEnabledFeatures stays null
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCI,
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = deviceExtensions,
    };
    vkcheck(vkCreateDevice(m_physicalDevice, &deviceCI, nullptr, &m_device));
    volkLoadDevice(m_device);

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