#include "4ZT/render/cpu-mt/cpu_renderer.h"
#include "4ZT/render/cpu-mt/cpu_intersect.h"
#include "4ZT/render/cpu-mt/cpu_scatter.h"

#include <iomanip>

FourZT::Image FourZT::CPURenderer::Render(const Settings& settings,
                                         const Camera& camera,
                                         const World& world) {
    // Determine the number of threads to use
    m_allowedThreads = std::min(m_maxAvailiableThreads, settings.targetThreads);

    int imgHeight = settings.outputHeight;
    int imgWidth = settings.outputHeight * settings.aspectRatio;
    Image img(imgWidth, imgHeight);
    
    // viewport setup
    // virtual grid where rays are casted from
    auto theta = torads(camera.fov);
    auto h = std::tan(theta/2);
    m_viewportHeight = 2 * h * camera.focalLength;
    m_viewportWidth  = m_viewportHeight * (double(imgWidth) / imgHeight);

    // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
    const auto vup = Vec3(0, 1, 0); // Up direction in world space
    auto w = (camera.position - camera.lookAt).normalize();
    auto u = cross(vup, w).normalize();
    auto v = cross(w, u);

    Vec3 viewport_u = m_viewportWidth * u;    // Vector across viewport horizontal edge
    Vec3 viewport_v = m_viewportHeight * v; 

    auto delta_u = viewport_u / imgWidth;
    auto delta_v = viewport_v / imgHeight;

    // Calculate the location of the upper left pixel.
    auto viewport_upper_left = camera.position - (camera.focalLength * w) - viewport_u/2 - viewport_v/2;
    auto pixelat00 = viewport_upper_left + (delta_u + delta_v) * 0.5;

    for (int y = 0; y < img.height(); ++y) {
        if (y % 10 == 0)
            std::cerr << "\rRendering: " << std::fixed << std::setprecision(1) << (100.0 * y / img.height()) << "% " << std::flush;
        
        for (int x = 0; x < img.width(); ++x) {
            Color pixelColor = Color(0, 0, 0);
            /// random sampling TODO: implement different sampling strategies (jittered)
            for (int s = 0; s < settings.samplesPerPixel; ++s) {
                double offset_x = randomDouble() - 0.5;   // [-0.5, 0.5)
                double offset_y = randomDouble() - 0.5;

                auto pixel_sample = pixelat00 + delta_u * (x + offset_x) + delta_v * (y + offset_y);
                auto dir = (pixel_sample - camera.position).normalize();
                Ray ray(camera.position, dir);

                pixelColor += TraceRay(ray, world, settings.maxDepth) * (1.0 / settings.samplesPerPixel);
            }
            img.at(x, y) = pixelColor;
        }
    }

    std::cerr << "\rComplete.               " << std::endl;

    return img;
}

Color FourZT::CPURenderer::TraceRay(const Ray& ray, const World& world, int depth) {
    if (depth <= 0) 
        return Color(0, 0, 0);
    
    for (const auto& object : world.Get()) {
        HitRecord record;
        if (Intersector::Hit(*object, ray, Interval(0, infinity), record)) { 
            Ray scattered;
            Color attenuation;
            if (Scatterer::Scatter(*object->GetMaterial(), ray, record, attenuation, scattered)) {
                return hadamard(attenuation, TraceRay(scattered, world, depth - 1));
            }
            
            return Color(0.0, 0.0, 0.0);
        }
    }    

    // Skies, just for testing
    Vec3 unit_direction = ray.Direction().normalize();
    double t = 0.5 * (unit_direction.y + 1.0);
    return (1.0 - t) * Color(1.0, 1.0, 1.0) + t * Color(0.5, 0.7, 1.0);
}