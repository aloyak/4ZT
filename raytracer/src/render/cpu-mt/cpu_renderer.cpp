#include "4ZT/math/math.h"
#include "4ZT/render/cpu-mt/cpu_renderer.h"
#include "4ZT/render/cpu-mt/cpu_intersect.h"
#include "4ZT/render/cpu-mt/cpu_scatter.h"
#include "4ZT/render/cpu-mt/cpu_emitted.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>
#include <thread>

FourZT::CPURenderer::CPURenderer(const Settings& settings) {
    int hw = static_cast<int>(std::thread::hardware_concurrency());
    m_allowedThreads = std::max(1, std::min(hw, settings.targetThreads)); // hw can be 0

    m_imgHeight = settings.outputHeight;
    m_imgWidth  = static_cast<int>(std::lround(settings.outputHeight * settings.aspectRatio));

    m_invSamples = 1.0 / settings.samplesPerPixel;
}

FourZT::CPURenderer::ViewportInfo FourZT::CPURenderer::ComputeViewport(const Camera& camera) const {
    auto theta = torads(camera.fov);
    auto h = std::tan(theta/2);
    double viewportHeight = 2 * h * camera.focalLength;
    double viewportWidth  = viewportHeight * (double(m_imgWidth) / m_imgHeight);

    // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
    const auto vup = Vec3(0, 1, 0); // Up direction in world space
    auto w = (camera.position - camera.lookAt).normalize();
    auto u = cross(vup, w).normalize();
    auto v = cross(w, u);

    Vec3 viewport_u = viewportWidth * u;    // Vector across viewport horizontal edge
    Vec3 viewport_v = viewportHeight * v;

    ViewportInfo vp;
    vp.origin = camera.position;
    vp.deltaU = viewport_u / m_imgWidth;
    vp.deltaV = viewport_v / m_imgHeight;

    auto upperLeft = camera.position - (camera.focalLength * w) - viewport_u / 2 - viewport_v / 2;
    vp.pixel00 = upperLeft + (vp.deltaU + vp.deltaV) * 0.5;
    return vp;
}

FourZT::Image FourZT::CPURenderer::Render(const Settings& settings,
                                          const Camera& camera,
                                          const World& world) {
    Image img(m_imgWidth, m_imgHeight);

    const ViewportInfo vp = ComputeViewport(camera);

    BVH bvh{};
    if (settings.enableBVH)
        bvh = BVH::Build(world);

    std::atomic<int> nextRow(0);
    std::atomic<int> rowsDone(0);

    auto worker = [&](int threadId) {
        while (true) {
            int y = nextRow.fetch_add(1);
            if (y >= m_imgHeight) break;

            for (int x = 0; x < m_imgWidth; ++x) {
                Color pixelColor(0, 0, 0);
                /// random sampling TODO: implement different sampling strategies (jittered)
                for (int s = 0; s < settings.samplesPerPixel; ++s) {
                    double offset_x = RandomDouble() - 0.5;   // [-0.5, 0.5)
                    double offset_y = RandomDouble() - 0.5;

                    auto pixel_sample = vp.pixel00 + vp.deltaU * (x + offset_x) + vp.deltaV * (y + offset_y);
                    auto dir = (pixel_sample - vp.origin).normalize();
                    Ray ray(vp.origin, dir);

                    pixelColor += TraceRay(ray, world, camera, bvh, settings, settings.maxDepth);
                }
                img.at(x, y) = pixelColor * m_invSamples; // divide once not per sample
            }

            int done = rowsDone.fetch_add(1) + 1;
            if (threadId == 0)
                std::cerr << "\rRendering: " << std::fixed << std::setprecision(1)
                          << (100.0 * done / m_imgHeight) << "% " << std::flush;
        }
    };

    // Spawn N-1 helpers (none if N == 1), then the calling thread works too
    std::vector<std::thread> threads;
    threads.reserve(m_allowedThreads - 1);
    for (int t = 1; t < m_allowedThreads; ++t)
        threads.emplace_back(worker, t);

    // Make sure the current thread also does some work!
    worker(0);

    for (auto& thread : threads)
        thread.join(); // Wait for all threads to finish

    std::cerr << "\rComplete.               " << std::endl;

    return img;
}

Color FourZT::CPURenderer::TraceRay(const Ray& ray, const World& world, const Camera& cam,
                                    const BVH& bvh, const Settings& settings, int depth) const {
    if (depth <= 0)
        return Color(0, 0, 0);

    HitRecord record;
    if (Intersector::Hit(world, bvh, ray, Interval(0.001, infinity), record)) {
        Ray scattered;
        Color attenuation;

        Color emitted = Emitter::Emitted(*record.material);

        if (Scatterer::Scatter(*record.material, ray, record, attenuation, scattered)) {
            double survival = 1.0;

            if (settings.enableRussianRoulette) {
                int bounces = settings.maxDepth - depth;

                if (bounces >= settings.minDepth) {
                    // Use the brightest channel of the attenuation as survival chance.
                    // Dark surfaces die quickly, bright ones live longer.
                    survival = std::max({attenuation.x, attenuation.y, attenuation.z});
                    survival = std::clamp(survival, 0.05, 0.95);

                    if (RandomDouble() >= survival)
                        return emitted;
                }
            }

            Color incoming = TraceRay(scattered, world, cam, bvh, settings, depth - 1);
            return emitted + hadamard(attenuation, incoming) / survival;
        }

        return emitted;
    }

    if (!cam.useSkybox)
        return cam.background;

    Vec3 unit_direction = ray.Direction().normalize();
    double t = 0.5 * (unit_direction.y + 1.0);
    return (1.0 - t) * Color(1.0, 1.0, 1.0) + t * Color(0.5, 0.7, 1.0);
}