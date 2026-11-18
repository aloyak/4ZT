#pragma once

#include "4ZT/math/math.h"

namespace FourZT {

class Ray {
public:
    Ray() = default;
    Ray(const Point3& origin, const Vec3& direction)
        : m_origin(origin), m_direction(direction) {}

    Point3 Origin() const { return m_origin; }
    Vec3 Direction() const { return m_direction; }

    Point3 At(double t) const {
        return m_origin + m_direction * t;
    }

private:
    Point3 m_origin;
    Vec3 m_direction;
};

}
