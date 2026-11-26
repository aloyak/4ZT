#pragma once

#include "4ZT/render/gpu-vulkan/gpu_types.h"
#include "4ZT/world/world.h"

#include <vector>

namespace FourZT {

// The scene vector needs to be "translated" into a friendly format
struct GpuSceneData {
    std::vector<GpuPrimitive> primitives;
    std::vector<GpuMaterial>  materials;
};

GpuSceneData BuildGpuScene(const World& world);

} // namespace FourZT