#pragma once

#include "4ZT/render/gpu-vulkan/vulkan_context.h"
#include "4ZT/render/gpu-vulkan/vulkan_pipeline.h"

#include "4ZT/render/render.h"

#include "4ZT/settings.h"
#include "4ZT/camera.h"
#include "4ZT/world/world.h"
#include "4ZT/output/image.h"

namespace FourZT {

class VulkanRenderer : public Renderer {
public:
    VulkanRenderer(const Settings& settings);
    ~VulkanRenderer() override = default;

    Image Render(const Settings& settings,
                 const Camera& camera,
                 const World& world) override;

private:
    VulkanContext m_context;
    VulkanPipeline m_pipeline;

    int m_imgHeight = 0;
    int m_imgWidth = 0;
};

}