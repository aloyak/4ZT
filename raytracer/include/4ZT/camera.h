#pragma once

#include "4ZT/math/math.h"

namespace FourZT {

    struct ViewportInfo {
    Vec3 origin;     // camera position
    Vec3 pixel00;    // center of pixel x=0 y=0
    Vec3 deltaU;     // step per pixel horizontally
    Vec3 deltaV;     // step per pixel vertically
};

class Camera {
public:
    Camera(
        const Point3& position = Point3(0, 0, 0),
        const Point3& lookAt = Point3(0, 0, -1),
        double fov = 180.0
    ) {
        this->position = position;
        this->lookAt = lookAt;
        this->fov = fov;
    }

    Point3 position;
    Point3 lookAt;
    
    double fov;
    double focalLength = 1.0; // distance from the camera to the image plane

    Color background = Color(0,0,0);
    bool useSkybox = true;


    /// TODO: fix off-by-one errors when going from doubles get truncated into ints when gpu rendering (for height width)
    static ViewportInfo ComputeViewport(const Camera& camera, const Settings& settings) {
        double imgHeight = settings.outputHeight;
        double imgWidth = settings.outputHeight * settings.aspectRatio;

        auto theta = torads(camera.fov);
        auto h = std::tan(theta/2);
        double viewportHeight = 2 * h * camera.focalLength;
        double viewportWidth  = viewportHeight * (double(imgWidth) / imgHeight);

        // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
        const auto vup = Vec3(0, 1, 0); // Up direction in world space
        auto w = (camera.position - camera.lookAt).normalize();
        auto u = cross(vup, w).normalize();
        auto v = cross(w, u);

        Vec3 viewport_u = viewportWidth * u;    // Vector across viewport horizontal edge
        Vec3 viewport_v = viewportHeight * v;

        ViewportInfo vp;
        vp.origin = camera.position;
        vp.deltaU = viewport_u / imgWidth;
        vp.deltaV = viewport_v / imgHeight;

        auto upperLeft = camera.position - (camera.focalLength * w) - viewport_u / 2 - viewport_v / 2;
        vp.pixel00 = upperLeft + (vp.deltaU + vp.deltaV) * 0.5;
        return vp;
    }
};

} // 4zt
