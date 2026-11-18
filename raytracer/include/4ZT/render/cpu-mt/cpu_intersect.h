#pragma once

#include "4ZT/object.h"
#include "4ZT/render/cpu-mt/ray.h"

namespace FourZT {
    
#include "4ZT/math/math.h"

struct HitRecord {
    double t;
    Vec3 point;
    Vec3 normal;
    Color color;

    bool frontFace;

    // Makes the normal always point against the incoming ray
    void SetFaceNormal(const Ray& r, const Vec3& outwardNormal) {
        frontFace = dot(r.Direction(), outwardNormal) < 0;
        normal = frontFace ? outwardNormal : -outwardNormal;
    }
};

class Intersector {
public:
    static bool Hit(const Object& object, const Ray& ray, Interval range, HitRecord& record);

    // sphere intersection
    static bool SphereHit(const Sphere& sphere, const Ray& ray, Interval range, HitRecord& record);

    /// TODO: triangle, mesh...
};

} // 4zt