#include "4ZT/render/render.h"
#include "4ZT/render/cpu-mt/cpu_renderer.h"
#include "4ZT/render/gpu-vulkan/vulkan_renderer.h"

#include "4ZT/engine.h"

#include <stdexcept> 

std::unique_ptr<FourZT::Renderer> FourZT::Renderer::Create(const Settings& settings) {
    switch (settings.backend) {
        case Backend::CPU:
            return std::make_unique<CPURenderer>(settings);
        case Backend::Vulkan:
            return std::make_unique<VulkanRenderer>(settings);
        case Backend::HIPRT:
            throw std::runtime_error("HIP-RT backend not implemented yet!");
        default:
            throw std::runtime_error("Unsupported backend");
    }
}