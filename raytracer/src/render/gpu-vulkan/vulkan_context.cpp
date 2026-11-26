#include "4ZT/render/gpu-vulkan/vulkan_context.h"

#include <VkBootstrap.h>

namespace {

#ifndef NDEBUG
constexpr bool kEnableValidation = true;
#else
constexpr bool kEnableValidation = false;
#endif

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

void FourZT::VulkanContext::Initialize(const Settings& settings) {
    try {
        vkcheck(volkInitialize());

        vkb::InstanceBuilder instanceBuilder{ vkGetInstanceProcAddr };
        instanceBuilder
            .set_app_name("4ZT Vulkan Renderer")
            .require_api_version(1, 3, 0)
            .set_headless(); // no surface or swapchain extensions
        if (kEnableValidation) {
            instanceBuilder
                .request_validation_layers()
                .set_debug_callback(DebugCallback)
                .set_debug_messenger_severity(VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                              VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT);
        }
        vkb::Instance vkbInstance = Unwrap(instanceBuilder.build(), "Failed to create Vulkan instance");
        m_instance = vkbInstance.instance;
        m_debugMessenger = vkbInstance.debug_messenger;
        volkLoadInstance(m_instance);

        // Physical device
        VkPhysicalDeviceFeatures features10{};
        features10.shaderInt64 = VK_TRUE;

        VkPhysicalDeviceVulkan12Features features12{};
        features12.bufferDeviceAddress                         = VK_TRUE;
        features12.timelineSemaphore                           = VK_TRUE;
        features12.descriptorIndexing                          = VK_TRUE;
        features12.runtimeDescriptorArray                      = VK_TRUE;
        features12.descriptorBindingPartiallyBound             = VK_TRUE;
        features12.shaderSampledImageArrayNonUniformIndexing   = VK_TRUE;

        VkPhysicalDeviceVulkan13Features features13{};
        features13.synchronization2 = VK_TRUE;

        vkb::PhysicalDeviceSelector selector{ vkbInstance };   // headless: no surface
        if (settings.targetDeviceIndex < 0) { // "auto"
            selector
            .set_minimum_version(1, 3)
            .set_required_features(features10)
            .set_required_features_12(features12)
            .set_required_features_13(features13)
            .prefer_gpu_device_type(vkb::PreferredDeviceType::discrete);
        } else {
            selector
            .set_minimum_version(1, 3)
            .set_required_features(features10)
            .set_required_features_12(features12)
            .set_required_features_13(features13);

            auto devices = Unwrap(selector.select_devices(), "Failed to enumerate Vulkan devices");
            if (settings.targetDeviceIndex >= static_cast<int>(devices.size())) {
                throw std::runtime_error("Invalid target device index: " + std::to_string(settings.targetDeviceIndex));
            }
            selector.set_name(devices[settings.targetDeviceIndex].properties.deviceName);
        }
        vkb::PhysicalDevice vkbPhysical = Unwrap(selector.select(), "Failed to select a suitable GPU");
        m_physicalDevice = vkbPhysical.physical_device;

        // logical device
        vkb::Device vkbDevice = Unwrap(vkb::DeviceBuilder{ vkbPhysical }.build(), "Device creation");
        m_device = vkbDevice.device;
        volkLoadDevice(m_device);

        m_queue       = Unwrap(vkbDevice.get_queue(vkb::QueueType::compute), "Compute queue");
        m_queueFamily = Unwrap(vkbDevice.get_queue_index(vkb::QueueType::compute), "Compute queue family");

        // vma
        VmaVulkanFunctions fns{};
        fns.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
        fns.vkGetDeviceProcAddr   = vkGetDeviceProcAddr;

        VmaAllocatorCreateInfo allocatorInfo{};
        allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;
        allocatorInfo.instance         = m_instance;
        allocatorInfo.physicalDevice   = m_physicalDevice;
        allocatorInfo.device           = m_device;
        allocatorInfo.flags            = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
        allocatorInfo.pVulkanFunctions = &fns;
        vkcheck(vmaCreateAllocator(&allocatorInfo, &m_allocator));

        // command pool
        VkCommandPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
        poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = m_queueFamily;
        vkcheck(vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool));


    } catch (...) {
        Destroy();
        throw;
    }
}

void FourZT::VulkanContext::Destroy() {
    if (m_device != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(m_device);
        if (m_commandPool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(m_device, m_commandPool, nullptr);
            m_commandPool = VK_NULL_HANDLE;
        }
        if (m_allocator != VK_NULL_HANDLE) {
            vmaDestroyAllocator(m_allocator);
            m_allocator = VK_NULL_HANDLE;
        }

        vkDestroyDevice(m_device, nullptr);
        m_device = VK_NULL_HANDLE;
    }

    if (m_instance != VK_NULL_HANDLE) {
        if (m_debugMessenger != VK_NULL_HANDLE) {
            vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);
            m_debugMessenger = VK_NULL_HANDLE;
        }
        vkDestroyInstance(m_instance, nullptr);
        m_instance = VK_NULL_HANDLE;
    }

    m_physicalDevice = VK_NULL_HANDLE;
    m_queue          = VK_NULL_HANDLE;
    m_queueFamily    = 0;
}
