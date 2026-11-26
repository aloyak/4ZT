import common;
import rng;
import intersect;
import scatter;

[[vk::binding(0, 0)]] RWStructuredBuffer<float4> outputImage;
[[vk::binding(1, 0)]] StructuredBuffer<GpuPrimitive> prims;
[[vk::binding(2, 0)]] StructuredBuffer<GpuMaterial> materials;
[[vk::push_constant]] ConstantBuffer<Params> pc;

float3 Sky(float3 dir)
{
    if (pc.useSkybox == 0) return pc.background.xyz;
    float t = 0.5 * (normalize(dir).y + 1.0);
    return lerp(float3(1.0), float3(0.5, 0.7, 1.0), t);
}

// no recursion in SPIR-V
float3 TraceRay(Ray ray, inout Rng rng)
{
    float3 radiance   = float3(0.0);
    float3 throughput = float3(1.0);

    for (uint depth = 0; depth < pc.maxDepth; ++depth) {
        HitRecord rec;
        if (!HitWorld(prims, pc.primCount, ray, 0.001, 1e30, rec)) {
            radiance += throughput * Sky(ray.dir);
            break;
        }

        GpuMaterial mat = materials[rec.material];
        if (mat.kind == MAT_EMISSIVE)
            radiance += throughput * mat.data.xyz;

        float3 atten;
        Ray    scattered;
        if (!Scatter(mat, ray, rec, rng, atten, scattered))
            break;

        throughput *= atten;

        // Russian roulette on accumulated throughput
        if (pc.enableRR != 0 && depth >= pc.minDepth) {
            float p = clamp(max(throughput.x, max(throughput.y, throughput.z)), 0.05, 0.95);
            if (rng.Next() >= p) break;
            throughput /= p;
        }

        ray = scattered;
    }
    return radiance;
}

[shader("compute")]
[numthreads(8, 8, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    if (tid.x >= pc.width || tid.y >= pc.height) return;

    uint pixelIndex = tid.y * pc.width + tid.x;
    float3 sum = float3(0.0);

    for (uint i = 0; i < pc.sampleCount; ++i) {
        uint s = pc.sampleStart + i; // global index: every batch gets new random numbers
        Rng rng = MakeRng(pixelIndex, s);

        float offset_x = rng.Next() - 0.5;
        float offset_y = rng.Next() - 0.5;
        float3 pixel = pc.pixel00.xyz
                     + pc.deltaU.xyz * (float(tid.x) + offset_x)
                     + pc.deltaV.xyz * (float(tid.y) + offset_y);

        Ray ray;
        ray.origin = pc.origin.xyz;
        ray.dir = normalize(pixel - ray.origin);

        float3 c = TraceRay(ray, rng);
        if (any(isnan(c))) c = float3(0.0);
        sum += c;
    }

    // the CPU divides by the total spp at readback
    outputImage[pixelIndex] += float4(sum, 0.0);
}