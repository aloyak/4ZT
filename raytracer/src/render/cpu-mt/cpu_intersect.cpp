#include "4ZT/render/cpu-mt/cpu_intersect.h"

// stack is used to keep track of the nodes we intersected with and need to visit later
// size is the stack pointer, which points to the next free slot in the stack
struct NodeStack {
    int data[64]; // 64 is generous, enough for a BVH with 2^64 nodes (would require 2^64 objects to overflow, theoretically)
    int size = 0;

    // [i++] is different to [++i], one increments the pointer after the assignment, the other increments before the assignment
    // equivalent to stack[i] = 0; i++; but more efficient
    void push(int v) { data[size++] = v; }
    int  pop()       { return data[--size]; }
    bool empty() const { return size == 0; }
};

bool FourZT::Intersector::Hit(const World& world, const BVH& bvh, const Ray& ray, Interval range, HitRecord& record) {
    if (bvh.nodes.empty()) return WorldHit(world, ray, range, record);

    const auto& objects = world.Get();
    bool hitAny = false;
    double closest = range.max;
    HitRecord temp;

    NodeStack stack;
    stack.push(0); // start at root

    while (!stack.empty()) {
        const BVHNode& node = bvh.nodes[stack.pop()];
        if (!AABBHit(node.bounds, ray, Interval(range.min, closest)))
            continue;   // skips this node and its whole subtree cuz theres no intersection

        if (node.objectCount > 0) {   // leaf, real intersection!
            for (int i = 0; i < node.objectCount; i++) {
                const Object& obj = *objects[bvh.objectIndices[node.leftOrFirst + i]];
                if (ObjectHit(obj, ray, Interval(range.min, closest), temp)) {
                    hitAny = true;
                    closest = temp.t;
                    record = temp;
                }
            }
        } else { // interior
            stack.push(node.leftOrFirst); // left child
            stack.push(node.leftOrFirst + 1); // right
        }
    }
    
    return hitAny;
}

// hit without bvh
bool FourZT::Intersector::WorldHit(const World& world, const Ray& ray, Interval range, HitRecord& record) {
    bool hitAny = false;
    double closest = range.max;
    HitRecord temp;

    for (const auto& object : world.Get()) {
        if (ObjectHit(*object, ray, Interval(range.min, closest), temp)) {
            hitAny = true;
            closest = temp.t;
            record = temp;
        }
    }
    return hitAny;
}

bool FourZT::Intersector::AABBHit(const AABB& box, const Ray& ray, Interval range) {
    const Point3& o   = ray.Origin();
    const Vec3&   inv = ray.InvDirection();
    double tmin = range.min;
    double tmax = range.max;

    for (int i = 0; i < 3; ++i) {
        double t0 = (box[i].min - o[i]) * inv[i];
        double t1 = (box[i].max - o[i]) * inv[i];
        double lo = t0 < t1 ? t0 : t1;
        double hi = t0 < t1 ? t1 : t0;
        tmin = lo > tmin ? lo : tmin;
        tmax = hi < tmax ? hi : tmax;
    }
    return tmin < tmax;
}


bool FourZT::Intersector::ObjectHit(const Object& object, const Ray& ray, Interval range, HitRecord& record) {
    switch (object.Kind()) {
        case ObjectKind::Sphere:
            return SphereHit(static_cast<const Sphere&>(object), ray, range, record);
        case ObjectKind::Quad:
            return QuadHit(static_cast<const Quad&>(object), ray, range, record);
        case ObjectKind::Triangle:
            return TriangleHit(static_cast<const Triangle&>(object), ray, range, record);
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
    record.material = sphere.GetMaterial();
    return true;
}

static constexpr double Epsilon = 1e-8;
bool FourZT::Intersector::QuadHit(const Quad& quad, const Ray& ray, Interval range, HitRecord& record) {
    const Vec3& u = quad.U();
    const Vec3& v = quad.V();

    double denom = dot(quad.Normal(), ray.Direction());
    if (std::abs(denom) < Epsilon) return false;

    double D = dot(quad.Normal(), quad.Q());
    double t = (D - dot(quad.Normal(), ray.Origin())) / denom;
    if (!range.Surrounds(t)) return false;

    Point3 p = ray.At(t);
    Vec3 planar = p - quad.Q();
    double alpha = dot(quad.W(), cross(planar, v));
    double beta  = dot(quad.W(), cross(u, planar));

    if (alpha < 0.0 || alpha > 1.0 || beta < 0.0 || beta > 1.0) return false;

    record.t = t;
    record.point = p;
    record.SetFaceNormal(ray, quad.Normal());
    record.material = quad.GetMaterial();
    return true;
}

bool FourZT::Intersector::TriangleHit(const Triangle& triangle, const Ray& ray,
                                      Interval range, HitRecord& record) {
    const Point3& p0 = triangle.P0();
    Vec3 e1 = triangle.P1() - p0;
    Vec3 e2 = triangle.P2() - p0;

    Vec3 pvec = cross(ray.Direction(), e2);
    double det = dot(e1, pvec);
    if (std::abs(det) < 1e-12) return false;   // parallel to triangle

    double invDet = 1.0 / det;
    Vec3 tvec = ray.Origin() - p0;

    double u = dot(tvec, pvec) * invDet;
    if (u < 0.0 || u > 1.0) return false;

    Vec3 qvec = cross(tvec, e1);
    double v = dot(ray.Direction(), qvec) * invDet;
    if (v < 0.0 || u + v > 1.0) return false;

    double t = dot(e2, qvec) * invDet;
    if (!range.Surrounds(t)) return false;

    record.t = t;
    record.point = ray.At(t);
    record.SetFaceNormal(ray, cross(e1, e2).normalize());
    record.material = triangle.GetMaterial();
    return true;
}