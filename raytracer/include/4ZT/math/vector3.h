#pragma once

#include <cmath>

class Vec3 {
public:
    double x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(double x, double y, double z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& o) const { return Vec3(x + o.x, y + o.y, z + o.z); }
    Vec3 operator-(const Vec3& o) const { return Vec3(x - o.x, y - o.y, z - o.z); }
    Vec3 operator-() const { return Vec3(-x, -y, -z); }
    Vec3 operator*(double s) const { return Vec3(x * s, y * s, z * s); }
    Vec3 operator/(double s) const { return Vec3(x / s, y / s, z / s); }

    Vec3 operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
    Vec3 operator-=(const Vec3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
    Vec3 operator*=(double s) { x *= s; y *= s; z *= s; return *this; }
    Vec3 operator/=(double s) { x /= s; y /= s; z /= s; return *this; }

    double operator[](int i) const {
        return (i == 0) ? x : (i == 1) ? y : z;
    }

    double dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }

    Vec3 cross(const Vec3& o) const {
        return Vec3(y * o.z - z * o.y,
                       z * o.x - x * o.z,
                       x * o.y - y * o.x);
    }

    double length() const { return std::sqrt(dot(*this)); }
    double lengthSquared() const { return dot(*this); }

    Vec3 normalize() const {
        double len = length();
        return len > 0 ? *this / len : Vec3();
    }
};

using Point3 = Vec3; 
using Color = Vec3;

inline Vec3 operator*(double s, const Vec3& v) {
    return v * s;
}


inline double dot(const Vec3& a, const Vec3& b) {
    return a.dot(b);
}
inline Vec3 cross(const Vec3& a, const Vec3& b) {
    return a.cross(b);
}

// Hadamard product (component-wise multiplication)
inline Vec3 hadamard(const Vec3& a, const Vec3& b) {
    return Vec3(a.x * b.x, a.y * b.y, a.z * b.z);
}

// Uniform random in [0, 1) (thread-safe)
inline double randomDouble() {
    thread_local std::mt19937 gen(std::random_device{}());
    thread_local std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(gen);
}

inline Vec3 RandomUnitVector() {
    double a = randomDouble() * 2.0 * M_PI;
    double z = randomDouble() * 2.0 - 1.0;
    double r = std::sqrt(1 - z*z);
    return Vec3(r * std::cos(a), r * std::sin(a), z);
}

inline Vec3 Reflect(const Vec3& v, const Vec3& n) {
    return v - 2*dot(v,n)*n;
}

inline Vec3 Refract(const Vec3& uv, const Vec3& n, double etai_over_etat) {
    auto cos_theta = std::fmin(dot(-uv, n), 1.0);
    Vec3 r_out_perp =  etai_over_etat * (uv + cos_theta*n);
    Vec3 r_out_parallel = -std::sqrt(std::fabs(1.0 - r_out_perp.lengthSquared())) * n;
    return r_out_perp + r_out_parallel;
}

/// TODO: no rejection sampling
inline Vec3 RandomInUnitSphere() {
    while (true) {
        auto p = Vec3(randomDouble(), randomDouble(), randomDouble()) * 2.0 - Vec3(1, 1, 1);
        if (p.lengthSquared() >= 1) continue;
        return p;
    }
}