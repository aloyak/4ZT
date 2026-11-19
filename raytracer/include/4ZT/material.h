#pragma once

// Abstract material class, they only hold the properties and data
// Scatter is done by the different renderer types, each having its own implementation

#include "4ZT/math/math.h"

namespace FourZT {

enum class MaterialKind {
    Lambertian,
    Metal,
    Dielectric
};

class Material {
public:
    virtual ~Material() = default;

    MaterialKind Kind() const { return m_kind; }
protected:
    explicit Material(MaterialKind kind) : m_kind(kind) {}

private:
    MaterialKind m_kind;
};

class Lambertian : public Material {
public:
    Lambertian(const Color& albedo) : m_albedo(albedo), Material(MaterialKind::Lambertian) {}

    const Color& Albedo() const { return m_albedo; }

private:
    Color m_albedo;
};

class Metal : public Material {
public:
    Metal(const Color& albedo, double fuzz) : m_albedo(albedo), m_fuzz(fuzz), Material(MaterialKind::Metal) {}

    const Color& Albedo() const { return m_albedo; }
    double Fuzz() const { return m_fuzz; }

private:
    Color m_albedo;
    double m_fuzz;
};

class Dialectric : public Material {
public:
    Dialectric(double refractionIndex) : m_refractionIndex(refractionIndex), Material(MaterialKind::Dielectric) {}

    double RefractionIndex() const { return m_refractionIndex; }

private:
    double m_refractionIndex;
};

} // 4zt