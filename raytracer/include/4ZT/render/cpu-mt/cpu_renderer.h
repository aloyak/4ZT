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
    int m_allowedThreads;
    int m_imgHeight = 0;
    int m_imgWidth = 0;
    double m_invSamples;
};

}