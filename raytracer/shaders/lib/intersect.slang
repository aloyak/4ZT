import common;

void SetFaceNormal(inout HitRecord rec, Ray ray, float3 outward)
{
    rec.frontFace = dot(ray.dir, outward) < 0.0;
    rec.normal    = rec.frontFace ? outward : -outward;
}

bool HitSphere(GpuPrimitive s, Ray ray, float tMin, float tMax, inout HitRecord rec)
{
    float3 center = s.a.xyz;
    float  radius = s.a.w;

    float3 oc = center - ray.origin;
    float  a  = dot(ray.dir, ray.dir);
    float  h  = dot(ray.dir, oc);
    float  c  = dot(oc, oc) - radius * radius;

    float disc = h * h - a * c;
    if (disc < 0.0) return false;

    float sq   = sqrt(disc);
    float root = (h - sq) / a;
    if (root <= tMin || root >= tMax) {
        root = (h + sq) / a;
        if (root <= tMin || root >= tMax) return false;
    }

    rec.t        = root;
    rec.point    = ray.origin + root * ray.dir;
    rec.material = s.material;
    SetFaceNormal(rec, ray, (rec.point - center) / radius);
    return true;
}

static float Epsilon = 1e-6;
bool HitQuad(GpuPrimitive q, Ray ray, float tMin, float tMax, inout HitRecord rec)
{
    float3 Q = q.a.xyz, u = q.b.xyz, v = q.c.xyz, n = q.d.xyz, w = q.e.xyz;

    float denom = dot(n, ray.dir);
    if (abs(denom) < Epsilon) return false;

    float t = (dot(n, Q) - dot(n, ray.origin)) / denom;
    if (t <= tMin || t >= tMax) return false;

    float3 p      = ray.origin + t * ray.dir;
    float3 planar = p - Q;
    float  alpha  = dot(w, cross(planar, v));
    float  beta   = dot(w, cross(u, planar));
    if (alpha < 0.0 || alpha > 1.0 || beta < 0.0 || beta > 1.0) return false;

    rec.t        = t;
    rec.point    = p;
    rec.material = q.material;
    SetFaceNormal(rec, ray, n);
    return true;
}

// Möller–Trumbore algorithm
bool HitTriangle(GpuPrimitive t, Ray ray, float tMin, float tMax, inout HitRecord rec)
{
    float3 p0 = t.a.xyz;
    float3 e1 = t.b.xyz - p0;
    float3 e2 = t.c.xyz - p0;

    float3 pvec = cross(ray.dir, e2);
    float det = dot(e1, pvec);
    if (abs(det) < Epsilon) return false; // parallel to triangle

    float invDet = 1.0 / det;
    float3 tvec = ray.origin - p0;

    float u = dot(tvec, pvec) * invDet;
    if (u < 0.0 || u > 1.0) return false;

    float3 qvec = cross(tvec, e1);
    float v = dot(ray.dir, qvec) * invDet;
    if (v < 0.0 || u + v > 1.0) return false;

    float hitT = dot(e2, qvec) * invDet;
    if (hitT <= tMin || hitT >= tMax) return false;

    rec.t = hitT;
    rec.point = ray.origin + hitT * ray.dir;
    rec.material = t.material;
    SetFaceNormal(rec, ray, normalize(cross(e1, e2)));
    return true;
}

// Brute force over every primitive, same as Intersector::WorldHit
// in other words, no bvh
bool HitWorld(StructuredBuffer<GpuPrimitive> prims, uint count,
              Ray ray, float tMin, float tMax, out HitRecord rec)
{
    rec = (HitRecord)0;
    bool  hitAny  = false;
    float closest = tMax;

    for (uint i = 0; i < count; ++i) {
        GpuPrimitive p = prims[i];
        HitRecord tmp = (HitRecord)0;
        bool hit = false;
        switch (p.kind) {
            case PRIM_SPHERE:   hit = HitSphere(p, ray, tMin, closest, tmp); break;
            case PRIM_QUAD:     hit = HitQuad(p, ray, tMin, closest, tmp); break;
            case PRIM_TRIANGLE: hit = HitTriangle(p, ray, tMin, closest, tmp); break;
        }

        if (hit) {
            hitAny  = true;
            closest = tmp.t;
            rec     = tmp;
        }
    }
    return hitAny;
}