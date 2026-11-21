#pragma once

#include <memory>

#include "4ZT/material.h"

#include "4ZT/render/cpu-mt/cpu_intersect.h"
#include "4ZT/render/cpu-mt/ray.h"

namespace FourZT {

class Scatterer {
public:
    static bool Scatter(const Material& material, const Ray& rayIn,const HitRecord& record, Color& attenuation, Ray& scattered);

    static bool LambertianScatter(const Lambertian& material, const Ray& rayIn, const HitRecord& record, Color& attenuation, Ray& scattered);
    static bool MetalScatter(const Metal& material, const Ray& rayIn, const HitRecord& record, Color& attenuation, Ray& scattered);
    static bool DielectricScatter(const Dielectric& material, const Ray& rayIn, const HitRecord& record, Color& attenuation, Ray& scattered);
};

} // 4zt