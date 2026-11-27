static const uint PRIM_SPHERE   = 0;
static const uint PRIM_QUAD     = 1;
static const uint PRIM_TRIANGLE = 2;

static const uint MAT_LAMBERTIAN = 0;
static const uint MAT_METAL      = 1;
static const uint MAT_DIELECTRIC = 2;
static const uint MAT_EMISSIVE   = 3;

struct Ray {
    float3 origin;
    float3 dir;
};

struct HitRecord {
    float t;
    float3 point;
    float3 normal; // always faces against the ray, like your CPU SetFaceNormal
    bool frontFace;
    uint material;
};

struct GpuMaterial {
    float4 data; // xyz = albedo / emission, w = fuzz or IOR
    uint kind;
    uint pad0;
    uint pad1;
    uint pad2;
};

struct GpuPrimitive {
    float4 a, b, c, d, e;
    uint kind;
    uint material;
    uint pad0;
    uint pad1;
};

struct Params {
    float4 origin; 
    float4 pixel00; 
    float4 deltaU; 
    float4 deltaV; 
    float4 background;

    uint width; 
    uint height; 
    
    uint sampleStart; 
    uint sampleCount;
    uint primCount; 
    uint maxDepth; 
    uint minDepth; 
    uint useSkybox;
    uint enableRR; 
    
    uint pad0; uint pad1; uint pad2;
};