#pragma once

#include <thread>

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
    Image Render(const Settings& settings,
                 const Camera& camera,
                 const World& world) override;

    Color TraceRay(const Ray& ray, const World& world, const Camera& cam, const BVH& bvh, int depth = 0) const;
private:
    int m_maxAvailiableThreads = std::thread::hardware_concurrency();
    int m_allowedThreads;

    /// TODO: check if it'd be better to move this to the abstract render class
    // only if the gpu renderer uses it too 
    double m_viewportWidth;
    double m_viewportHeight;
};

}