#pragma once

// Abstract object class, they only hold the properties and data
// Intersection is done by the different renderer types, each having its own implementation

#include <memory>

#include "4ZT/math/math.h"
#include "4ZT/world/material.h"

namespace FourZT {

enum class ObjectKind {
    Sphere,
    Quad,
    Triangle
    // TODO: cylinder, cone, torus?
};

class Object {
public:
    virtual ~Object() = default;

    ObjectKind Kind() const { return m_kind; }

    inline const Material* GetMaterial() const { return m_material.get(); }
        
protected:
    explicit Object(ObjectKind kind, std::shared_ptr<Material> material) 
        : m_kind(kind), m_material(std::move(material)) {}
        
private:
    ObjectKind m_kind;
    std::shared_ptr<Material> m_material; 
};

class Sphere : public Object {
public:
    Sphere(const Point3& center, double radius, std::shared_ptr<Material> material)
        : Object(ObjectKind::Sphere, std::move(material)), m_center(center), m_radius(radius) 
    {}

    const Point3& Center() const { return m_center; }
    double Radius() const { return m_radius; }

private:
    Point3 m_center{};
    double m_radius = 0.0;
};

class Quad : public Object {
public:
    Quad(const Point3& Q, const Vec3& u, const Vec3& v, std::shared_ptr<Material> material)
        : Object(ObjectKind::Quad, std::move(material)), m_Q(Q), m_u(u), m_v(v)
    {
        Vec3 n = cross(u, v);
        m_normal = n.normalize();
        m_w = n / dot(n, n);
    }

    const Point3& Q() const { return m_Q; }
    const Vec3& U() const { return m_u; }
    const Vec3& V() const { return m_v; }

    Point3 P0() const { return m_Q; }
    Point3 P1() const { return m_Q + m_u; }
    Point3 P2() const { return m_Q + m_u + m_v; }
    Point3 P3() const { return m_Q + m_v; }

    const Vec3& Normal() const { return m_normal; }
    const Vec3& W() const { return m_w; }

private:
    Point3 m_Q;
    Vec3 m_u, m_v;

    // precomputed for efficency, once per quad instead of per ray
    Vec3 m_normal;
    Vec3 m_w;
};

class Triangle : public Object {
public:
    Triangle(const Point3& p0, const Point3& p1, const Point3& p2, std::shared_ptr<Material> material)
        : Object(ObjectKind::Triangle, std::move(material)), m_p0(p0), m_p1(p1), m_p2(p2)
    {
        Vec3 n = cross(p1 - p0, p2 - p0);
        m_normal = n.normalize();
        m_w = n / dot(n, n);
    }

    const Point3& P0() const { return m_p0; }
    const Point3& P1() const { return m_p1; }
    const Point3& P2() const { return m_p2; }

    const Vec3& Normal() const { return m_normal; }
    const Vec3& W() const { return m_w; }

private:
    Point3 m_p0, m_p1, m_p2;
    Vec3 m_normal;
    Vec3 m_w;
};

} // 4zt