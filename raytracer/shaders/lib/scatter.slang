import common;
import rng;

float Reflectance(float cosine, float ri)
{
    float r0 = (1.0 - ri) / (1.0 + ri);
    r0 = r0 * r0;
    return r0 + (1.0 - r0) * pow(1.0 - cosine, 5.0);
}

bool Scatter(GpuMaterial mat, Ray rayIn, HitRecord rec, inout Rng rng,
             out float3 attenuation, out Ray scattered)
{
    attenuation = float3(1.0);
    scattered.origin = rec.point;
    scattered.dir    = float3(0.0);

    if (mat.kind == MAT_LAMBERTIAN) {
        float3 dir = rec.normal + rng.UnitVector();
        if (dot(dir, dir) < 1e-12) dir = rec.normal; // degenerate case
        scattered.dir = dir;
        attenuation   = mat.data.xyz;
        return true;
    }

    if (mat.kind == MAT_METAL) {
        float3 refl = reflect(normalize(rayIn.dir), rec.normal);
        refl += mat.data.w * rng.UnitVector();
        if (dot(refl, rec.normal) <= 0.0) return false; // fuzz pushed it into the surface
        scattered.dir = refl;
        attenuation   = mat.data.xyz;
        return true;
    }

    if (mat.kind == MAT_DIELECTRIC) {
        float  ior = mat.data.w;
        float  ri  = rec.frontFace ? (1.0 / ior) : ior;
        float3 unitDir = normalize(rayIn.dir);

        float cosTheta = min(dot(-unitDir, rec.normal), 1.0);
        float sinTheta = sqrt(max(0.0, 1.0 - cosTheta * cosTheta));

        bool cannotRefract = ri * sinTheta > 1.0;
        float3 dir;
        if (cannotRefract || Reflectance(cosTheta, ri) > rng.Next()) {
            dir = reflect(unitDir, rec.normal);
        } else {
            float3 perp     = ri * (unitDir + cosTheta * rec.normal);
            float3 parallel = -sqrt(abs(1.0 - dot(perp, perp))) * rec.normal;
            dir = perp + parallel;
        }
        scattered.dir = dir;
        return true;
    }

    return false;   // emissive and unknown kinds don't scatter
}