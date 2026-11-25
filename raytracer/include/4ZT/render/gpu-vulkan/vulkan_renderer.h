#pragma once

#include "4ZT/render/render.h"

#include "4ZT/settings.h"
#include "4ZT/camera.h"
#include "4ZT/world/world.h"
#include "4ZT/output/image.h"

#include "4ZT/bvh/bvh.h"

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1

#include <volk.h>
#include <vk_mem_alloc.h>

namespace FourZT {

class VulkanRenderer : public Renderer {
public:
    VulkanRenderer(const Settings& settings) {}
    ~VulkanRenderer() override {}

    Image Render(const Settings& settings,
                 const Camera& camera,
                 const World& world) override;

private:
    void Initialize();
    void Destroy();
};

}