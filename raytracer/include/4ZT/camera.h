#pragma once

#include "4ZT/math/math.h"

namespace FourZT {

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
};

} // 4zt
