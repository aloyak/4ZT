#pragma once

#include <memory>

#include "4ZT/math/math.h"
#include "4ZT/material.h"

namespace FourZT {

enum class ObjectKind {
    Sphere,
    Triangle,
};

class Object {
public:
    virtual ~Object() = default;

    ObjectKind Kind() const { return m_kind; }

    std::shared_ptr<Material> GetMaterial() const { return m_material; }
        
protected:
    explicit Object(ObjectKind kind, std::shared_ptr<Material> material) 
        : m_kind(kind), m_material(std::move(material)) {}
        
private:
    std::shared_ptr<Material> m_material; 
    ObjectKind m_kind;
};

class Sphere : public Object {
public:
    Sphere(const Point3& center, double radius, std::shared_ptr<Material> material)
        : Object(ObjectKind::Sphere, std::move(material)),
          m_center(center), m_radius(radius) {}

    const Point3& Center() const { return m_center; }
    double Radius() const { return m_radius; }

private:
    Point3 m_center{};
    double m_radius = 0.0;
};

/// TODO: plane, triangle, mesh...

} // 4zt