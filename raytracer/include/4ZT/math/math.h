#pragma once

#include <cmath>
#include <random>

#include "4ZT/math/vector3.h"
#include "4ZT/math/interval.h"

const double infinity = std::numeric_limits<double>::infinity();

inline double torads(double degrees) {
    return degrees * M_PI / 180.0;
}

inline double todegs(double radians) {
    return radians * 180.0 / M_PI;
}

static double Reflectance(double cosine, double refraction_index) {
    // Use Schlick's approximation for reflectance.
    auto r0 = (1 - refraction_index) / (1 + refraction_index);
    r0 = r0*r0;
    return r0 + (1-r0)*std::pow((1 - cosine),5);
}