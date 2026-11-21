#pragma once

#include "4ZT/math/math.h"

namespace FourZT {

class Ray {
public:
    Ray() = default;
    Ray(const Point3& origin, const Vec3& direction)
        : m_origin(origin), m_direction(direction),
          m_invDir(1.0 / direction.x, 1.0 / direction.y, 1.0 / direction.z) {}

    const Point3& Origin() const { return m_origin; }
    const Vec3& Direction() const { return m_direction; }
    const Vec3& InvDirection() const { return m_invDir; }

    Point3 At(double t) const { 
        return m_origin + m_direction * t; 
    }

private:
    Point3 m_origin;
    Vec3 m_direction;
    Vec3 m_invDir;
};

}
