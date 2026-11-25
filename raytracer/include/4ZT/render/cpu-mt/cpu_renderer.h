#pragma once

#include "4ZT/render/render.h"
#include "4ZT/render/cpu-mt/ray.h"

#include "4ZT/settings.h"
#include "4ZT/camera.h"
#include "4ZT/world/world.h"
#include "4ZT/output/image.h"

#include "4ZT/bvh/bvh.h"

namespace FourZT {

class CPURenderer : public Renderer {
public:
    CPURenderer(const Settings& settings);

    Image Render(const Settings& settings,
                 const Camera& camera,
                 const World& world) override;

    Color TraceRay(const Ray& ray, const World& world, const Camera& cam,
                   const BVH& bvh, const Settings& settings, int depth = 0) const;

private:
    struct ViewportInfo {
        Vec3 origin;     // camera position
        Vec3 pixel00;    // center of pixel x=0 y=0
        Vec3 deltaU;     // step per pixel horizontally
        Vec3 deltaV;     // step per pixel vertically
    };

    ViewportInfo ComputeViewport(const Camera& camera) const;

    // static
    int m_allowedThreads;
    int m_imgHeight;
    int m_imgWidth;
    double m_invSamples;
};

}