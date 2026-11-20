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
    if (bvh.nodes.empty()) return false;

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

bool FourZT::Intersector::AABBHit(const AABB& box, const Ray& ray, Interval range) {
    for (int i = 0; i < 3; ++i) {
        double invDir = 1.0 / ray.Direction()[i];
        double t0 = (box[i].min - ray.Origin()[i]) * invDir;
        double t1 = (box[i].max - ray.Origin()[i]) * invDir;
        if (invDir < 0.0) std::swap(t0, t1);
        range.min = std::max(t0, range.min);
        range.max = std::min(t1, range.max);
        if (range.max <= range.min)
            return false;
    }
    return true;
}


bool FourZT::Intersector::ObjectHit(const Object& object, const Ray& ray, Interval range, HitRecord& record) {
    switch (object.Kind()) {
        case ObjectKind::Sphere:
            return SphereHit(static_cast<const Sphere&>(object), ray, range, record);
        case ObjectKind::Quad:
            return QuadHit(static_cast<const Quad&>(object), ray, range, record);
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
    record.material = sphere.GetMaterial();
    return true;
}

bool FourZT::Intersector::QuadHit(const Quad& quad, const Ray& ray, Interval range, HitRecord& record) {
    const Vec3& u = quad.U();
    const Vec3& v = quad.V();

    Vec3 n = cross(u, v);
    Vec3 normal = n.normalize();
    Vec3 w = n / dot(n, n);              // precomputable per quad

    double denom = dot(normal, ray.Direction());
    if (std::abs(denom) < 1e-8) return false;

    double D = dot(normal, quad.Q());
    double t = (D - dot(normal, ray.Origin())) / denom;
    if (!range.Surrounds(t)) return false;

    Point3 p = ray.At(t);
    Vec3 planar = p - quad.Q();
    double alpha = dot(w, cross(planar, v));
    double beta  = dot(w, cross(u, planar));

    if (alpha < 0.0 || alpha > 1.0 || beta < 0.0 || beta > 1.0) return false;

    record.t = t;
    record.point = p;
    record.SetFaceNormal(ray, normal);
    record.material = quad.GetMaterial();
    return true;
}