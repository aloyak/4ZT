#include "4ZT/render/cpu-mt/cpu_intersect.h"

bool FourZT::Intersector::Hit(const Object& object, const Ray& ray, Interval range, HitRecord& record) {
    switch (object.Kind()) {
        case ObjectKind::Sphere:
            return SphereHit(static_cast<const Sphere&>(object), ray, range, record);
        //case ObjectKind::Triangle:
        //case ObjectKind::Mesh:
        default:
            return false;
    }
}
bool FourZT::Intersector::SphereHit(const Sphere& sphere, const Ray& ray, Interval range, HitRecord& record) {
    Vec3 oc = sphere.Center() - ray.Origin();
    double a = ray.Direction().lengthSquared();
    double h = dot(ray.Direction(), oc);          // half of b
    double c = oc.lengthSquared() - sphere.Radius() * sphere.Radius();

    double discriminant = h * h - a * c;
    if (discriminant < 0.0)
        return false;

    double sqrtd = std::sqrt(discriminant);

    // Find the nearest root within the acceptable range
    double root = (h - sqrtd) / a;
    if (!range.Surrounds(root)) {
        root = (h + sqrtd) / a;
        if (!range.Surrounds(root))
            return false;
    }

    record.t = root;
    record.point = ray.At(root);
    Vec3 outwardNormal = (record.point - sphere.Center()) / sphere.Radius();
    record.SetFaceNormal(ray, outwardNormal);
    return true;
}