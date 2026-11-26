#pragma once
#include <cstdint>

namespace FourZT {

constexpr uint32_t kGpuPrimSphere = 0;
constexpr uint32_t kGpuPrimQuad   = 1;

constexpr uint32_t kGpuMatLambertian = 0;
constexpr uint32_t kGpuMatMetal      = 1;
constexpr uint32_t kGpuMatDielectric = 2;
constexpr uint32_t kGpuMatEmissive   = 3;

struct GpuMaterial { // 32 bytes
    float    data[4];
    uint32_t kind;
    uint32_t pad[3];
};

struct GpuPrimitive { // 96 bytes
    float    a[4];                // sphere: center.xyz, radius. quad: Q
    float    b[4];                //                             quad: u
    float    c[4];                //                             quad: v
    float    d[4];                //                             quad: normal
    float    e[4];                //                             quad: w
    uint32_t kind;
    uint32_t material;            // index into the material buffer
    uint32_t pad[2];
};

// push constants are limited to 128 bytes!
static_assert(sizeof(GpuMaterial)  == 32);
static_assert(sizeof(GpuPrimitive) == 96);

struct PushConstants { // 128 bytes
    float    origin[4];
    float    pixel00[4];
    float    deltaU[4];
    float    deltaV[4];
    float    background[4];
    uint32_t width, height, sampleStart, sampleCount;   // sampleCount = this batch only
    uint32_t primCount, maxDepth, minDepth, useSkybox;
    uint32_t enableRR, pad0, pad1, pad2;
};
static_assert(sizeof(PushConstants) == 128);

} // 4zt