#pragma once

#include "4ZT/math/math.h"
#include "4ZT/world/object.h"

namespace FourZT {

class AABB {
public:
    Interval x, y, z;

    AABB() {}

    AABB(const Interval& x, const Interval& y, const Interval& z)
        : x(x), y(y), z(z) { PadToMinimums(); }

    AABB(const Point3& min, const Point3& max)
        : x(min.x, max.x), y(min.y, max.y), z(min.z, max.z) { PadToMinimums(); }

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
            case ObjectKind::Quad: {
                const auto& q = static_cast<const Quad&>(object);
                Point3 min(
                    std::min({q.P0().x, q.P1().x, q.P2().x, q.P3().x}),
                    std::min({q.P0().y, q.P1().y, q.P2().y, q.P3().y}),
                    std::min({q.P0().z, q.P1().z, q.P2().z, q.P3().z})
                );
                Point3 max(
                    std::max({q.P0().x, q.P1().x, q.P2().x, q.P3().x}),
                    std::max({q.P0().y, q.P1().y, q.P2().y, q.P3().y}),
                    std::max({q.P0().z, q.P1().z, q.P2().z, q.P3().z})
                );
                return AABB(min, max);
            }
            case ObjectKind::Triangle: {
                const auto& t = static_cast<const Triangle&>(object);
                Point3 min(
                    std::min({t.P0().x, t.P1().x, t.P2().x}),
                    std::min({t.P0().y, t.P1().y, t.P2().y}),
                    std::min({t.P0().z, t.P1().z, t.P2().z})
                );
                Point3 max(
                    std::max({t.P0().x, t.P1().x, t.P2().x}),
                    std::max({t.P0().y, t.P1().y, t.P2().y}),
                    std::max({t.P0().z, t.P1().z, t.P2().z})
                );
                return AABB(min, max);
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

private:
    // Fix for quad's bounding boxes that make sure bounding boxes are 
    // not degenerate (zero size) in any dimension, which can cause issues
    // if the quad is perfectly aligned with one of the axes
    void PadToMinimums() {
        const double epsilon = 1e-4;
        if (x.max - x.min < epsilon) { x.min -= epsilon * 0.5; x.max += epsilon * 0.5; }
        if (y.max - y.min < epsilon) { y.min -= epsilon * 0.5; y.max += epsilon * 0.5; }
        if (z.max - z.min < epsilon) { z.min -= epsilon * 0.5; z.max += epsilon * 0.5; }
    }

};

} // 4zt