// vulkan output test shader, not used currently

struct Params { uint width; uint height; };

// This is the output image buffer, where every pixel's color will be written to
[[vk::binding(0)]] RWStructuredBuffer<float4> outputImage;
//* "pc" is a push constant buffer, so we dont have to bind it to a descriptor set
[[vk::push_constant]] ConstantBuffer<Params> pc;

[shader("compute")]
[numthreads(8, 8, 1)] // 8x8 2D thread groups, 64 threads per group
void main(uint3 tid: SV_DispatchThreadID) {
    if (tid.x >= pc.width || tid.y >= pc.height) return; // dispatch is rounded up to 8x8 groups

    float u = (float(tid.x) + 0.5) / float(pc.width);
    float v = (float(tid.y) + 0.5) / float(pc.height);
    outputImage[tid.y * pc.width + tid.x] = float4(u, v, 0.2, 1.0);
}