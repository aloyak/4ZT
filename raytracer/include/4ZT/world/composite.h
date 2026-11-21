#pragma once

#include <cmath>
#include <memory>
#include <vector>

#include "4ZT/math/math.h"
#include "4ZT/world/object.h"

namespace FourZT {

struct Transform {
    Point3 position{0, 0, 0};
    Vec3   rotation{0, 0, 0};   // Euler angles in radians, applied X, then Y, then Z
    Vec3   scale{1, 1, 1};

    // applies scale and rotation
    Vec3 TransformVector(const Vec3& v) const {
        double x = v.x * scale.x, y = v.y * scale.y, z = v.z * scale.z;

        double cx = std::cos(rotation.x), sx = std::sin(rotation.x);
        double y1 = y * cx - z * sx;
        double z1 = y * sx + z * cx;
        y = y1; z = z1;

        double cy = std::cos(rotation.y), sy = std::sin(rotation.y);
        double x2 = x * cy + z * sy;
        double z2 = -x * sy + z * cy;
        x = x2; z = z2;

        double cz = std::cos(rotation.z), sz = std::sin(rotation.z);
        double x3 = x * cz - y * sz;
        double y3 = x * sz + y * cz;

        return Vec3(x3, y3, z);
    }

    // applies translation
    Point3 TransformPoint(const Point3& p) const {
        return position + TransformVector(Vec3(p.x, p.y, p.z));
    }
};

// Objects made out of other primitives
// They are built in local space and placed with a Transform.
class Composite {
public:
    virtual ~Composite() = default;

    Composite& SetPosition(const Point3& p) { m_transform.position = p; return *this; }
    Composite& SetRotation(const Vec3& r)   { m_transform.rotation = r; return *this; }
    Composite& SetScale(const Vec3& s)      { m_transform.scale = s;    return *this; }

    std::vector<std::unique_ptr<Object>> GetPrimitives() const {
        std::vector<std::unique_ptr<Object>> out;
        Build(out);
        return out;
    }

protected:
    virtual void Build(std::vector<std::unique_ptr<Object>>& out) const = 0;

    void AddQuad(std::vector<std::unique_ptr<Object>>& out,
                 const Point3& q, const Vec3& u, const Vec3& v,
                 const std::shared_ptr<Material>& material) const {
        out.push_back(std::make_unique<Quad>(
            m_transform.TransformPoint(q),
            m_transform.TransformVector(u),
            m_transform.TransformVector(v),
            material));
    }

    //void AddTriangle(); // (for meshes)

    Transform m_transform;
};

// Unit cube centered on the origin in local space
// position = center, scale = size, rotation = orientation
class Box : public Composite {
public:
    Box(const Point3& center, const Vec3& size, const Vec3& rotation,
        std::shared_ptr<Material> material)
        : m_material(std::move(material)) {
        m_transform.position = center;
        m_transform.scale    = size;
        m_transform.rotation = rotation;
    }

protected:
    void Build(std::vector<std::unique_ptr<Object>>& out) const override {
        const double h = 0.5;
        const Vec3 X(1, 0, 0), Y(0, 1, 0), Z(0, 0, 1);

        AddQuad(out, Point3(-h, -h, -h), Y, X, m_material); // -Z
        AddQuad(out, Point3(-h, -h,  h), X, Y, m_material); // +Z
        AddQuad(out, Point3(-h, -h, -h), Z, Y, m_material); // -X
        AddQuad(out, Point3( h, -h, -h), Y, Z, m_material); // +X
        AddQuad(out, Point3(-h, -h, -h), X, Z, m_material); // -Y
        AddQuad(out, Point3(-h,  h, -h), Z, X, m_material); // +Y
    }

private:
    std::shared_ptr<Material> m_material;
};

} // 4zt