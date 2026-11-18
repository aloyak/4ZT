#include "4ZT/render/cpu-mt/cpu_scatter.h"

bool FourZT::Scatterer::Scatter(
    const Material& material, const Ray& rayIn,
    const HitRecord& record, Color& attenuation, Ray& scattered
) {

    switch (material.Kind()) {
        case MaterialKind::Lambertian:
            return LambertianScatter(static_cast<const Lambertian&>(material),
                                     rayIn, record, attenuation, scattered);
        case MaterialKind::Metal:
            return MetalScatter(static_cast<const Metal&>(material),
                                rayIn, record, attenuation, scattered);
        case MaterialKind::Dielectric:
            return DialectricScatter(static_cast<const Dialectric&>(material),
                                     rayIn, record, attenuation, scattered);
        default:
            return false;
    }
}

bool FourZT::Scatterer::LambertianScatter(
    const Lambertian& material, const Ray& rayIn, const HitRecord& record, Color& attenuation, Ray& scattered
) {
    auto dir = record.normal + RandomUnitVector();
    scattered = Ray(record.point, dir);
    attenuation = material.Albedo();
    return true;
}

bool FourZT::Scatterer::MetalScatter(
    const Metal& material, const Ray& rayIn, const HitRecord& record, Color& attenuation, Ray& scattered
) {
    Vec3 reflected = Reflect(rayIn.Direction(), record.normal);
    reflected += material.Fuzz() * RandomUnitVector();
    scattered = Ray(record.point, reflected);
    attenuation = material.Albedo();
    return true;
}

bool FourZT::Scatterer::DialectricScatter(
    const Dialectric& material, const Ray& rayIn, const HitRecord& record, Color& attenuation, Ray& scattered
) {
    attenuation = Color(1.0, 1.0, 1.0);
    double ri = record.frontFace ? (1.0/material.RefractionIndex()) : material.RefractionIndex();

    Vec3 unit_direction = rayIn.Direction().normalize();
    double cos_theta = std::fmin(dot(-unit_direction, record.normal), 1.0);
    double sin_theta = std::sqrt(1.0 - cos_theta*cos_theta);

    bool cannotRefract = ri * sin_theta > 1.0;
    Vec3 direction;

    if (cannotRefract || Reflectance(cos_theta, ri) > randomDouble())
        direction = Reflect(unit_direction, record.normal);
    else
        direction = Refract(unit_direction, record.normal, ri);

    scattered = Ray(record.point, direction);
    return true;
}