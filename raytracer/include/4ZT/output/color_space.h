#pragma once
#include <algorithm>
#include <cmath>

namespace FourZT::ColorSpace {

// Exact sRGB encoding (piecewise)
inline float LinearToSRGB(float c) {
    c = std::clamp(c, 0.0f, 1.0f);
    return c <= 0.0031308f ? 12.92f * c
                           : 1.055f * std::pow(c, 1.0f / 2.4f) - 0.055f;
}

// cheap approximation to gamma 2.2
inline float LinearToGamma2(float c) {
    return std::sqrt(std::clamp(c, 0.0f, 1.0f));
}

inline float SRGBToLinear(float c) {
    return c <= 0.04045f ? c / 12.92f
                         : std::pow((c + 0.055f) / 1.055f, 2.4f);
}

} // 4zt