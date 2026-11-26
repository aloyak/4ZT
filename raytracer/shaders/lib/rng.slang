// "scrambler" it gives a "random" looking value from a given input
// simple PCG hash function
uint PcgHash(uint v)
{
    uint state = v * 747796405u + 2891336453u;
    uint word  = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

struct Rng
{
    uint state;

    [mutating]
    float Next() // [0, 1)
    {
        state = PcgHash(state);
        return float(state >> 8) * (1.0 / 16777216.0);
    }

    [mutating]
    float3 UnitVector() // uniform on the sphere, no rejection loop
    {
        float z   = 2.0 * Next() - 1.0;
        float phi = 6.2831853 * Next();
        float r   = sqrt(max(0.0, 1.0 - z * z));
        return float3(r * cos(phi), r * sin(phi), z);
    }
};

Rng MakeRng(uint pixelIndex, uint sampleIndex)
{
    Rng r;
    r.state = PcgHash(pixelIndex ^ PcgHash(sampleIndex + 0x9e3779b9u));
    return r;
}