#pragma once

#include "4ZT/math/math.h"
#include "4ZT/object.h"

namespace FourZT {

class AABB {
public:
    Interval x, y, z;

    AABB() {}

    AABB(const Interval& x, const Interval& y, const Interval& z) 
        : x(x), y(y), z(z) {}

    AABB(const Point3& min, const Point3& max) 
        : x(min.x, max.x), y(min.y, max.y), z(min.z, max.z) {}

    // new bounding box that contains other two bounding boxes
    AABB(const AABB& a, const AABB& b) 
        : x(a.x, b.x), y(a.y, b.y), z(a.z, b.z) {}

    const Interval& operator[](int i) const {
        return (i == 0) ? x : (i == 1) ? y : z;
    }


    static AABB BoundingBox(const Object& object) {
        switch (object.Kind()) {
            case ObjectKind::Sphere: {
                const auto& s = static_cast<const Sphere&>(object);
                Vec3 r(s.Radius(), s.Radius(), s.Radius());
                return AABB(s.Center() - r, s.Center() + r);
            }
            default:
                return AABB();   // empty box
        }
    }

    // Helpers
    Point3 Centroid() const {
        return Point3((x.min + x.max) * 0.5, (y.min + y.max) * 0.5, (z.min + z.max) * 0.5);
    }

    int LongestAxis() const {
        double sx = x.max - x.min, sy = y.max - y.min, sz = z.max - z.min;
        return (sx > sy && sx > sz) ? 0 : (sy > sz ? 1 : 2);
    }
};

} // 4zt