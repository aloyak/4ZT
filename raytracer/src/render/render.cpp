#include "4ZT/render/render.h"
#include "4ZT/render/cpu-mt/cpu_renderer.h"
//#include "4ZT/render/vulkan_renderer.h"

#include "4ZT/engine.h"

#include <stdexcept> 

std::unique_ptr<FourZT::Renderer> FourZT::Renderer::Create(const Settings& settings) {
    switch (settings.backend) {
        case Backend::CPU:
            return std::make_unique<CPURenderer>();
        case Backend::GPU:
            //return std::make_unique<VulkanRenderer>();
            throw std::runtime_error("GPU backend not implemented yet");
        default:
            throw std::runtime_error("Unsupported backend");
    }
}