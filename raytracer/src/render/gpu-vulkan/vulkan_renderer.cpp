#include "4ZT/render/gpu-vulkan/vulkan_renderer.h"

#include <VkBootstrap.h>

void FourZT::VulkanRenderer::Initialize() {

}

void FourZT::VulkanRenderer::Destroy() {

}

FourZT::Image FourZT::VulkanRenderer::Render(const Settings& settings,
                                 const Camera& camera,
                                 const World& world) {

    int imgHeight = settings.outputHeight;
    int imgWidth = settings.outputHeight * settings.aspectRatio;
    Image img(imgWidth, imgHeight);

    return img;
}