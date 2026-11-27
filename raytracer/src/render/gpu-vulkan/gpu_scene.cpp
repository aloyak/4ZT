#include "4ZT/render/gpu-vulkan/gpu_scene.h"

#include <stdexcept>
#include <unordered_map>

namespace {

using namespace FourZT;

void Set4(float (&dst)[4], double x, double y, double z, double w = 0.0) {
    dst[0] = float(x); dst[1] = float(y); dst[2] = float(z); dst[3] = float(w);
}

GpuMaterial ToGpuMaterial(const Material& m) {
    GpuMaterial g{};
    switch (m.Kind()) {
        case MaterialKind::Lambertian: {
            const auto& l = static_cast<const Lambertian&>(m);
            g.kind = kGpuMatLambertian;
            Set4(g.data, l.Albedo().x, l.Albedo().y, l.Albedo().z);
            break;
        }
        case MaterialKind::Metal: {
            const auto& mt = static_cast<const Metal&>(m);
            g.kind = kGpuMatMetal;
            Set4(g.data, mt.Albedo().x, mt.Albedo().y, mt.Albedo().z, mt.Fuzz());
            break;
        }
        case MaterialKind::Dielectric: {
            const auto& d = static_cast<const Dielectric&>(m);
            g.kind = kGpuMatDielectric;
            Set4(g.data, 1, 1, 1, d.RefractionIndex());
            break;
        }
        case MaterialKind::Emissive: {
            const auto& e = static_cast<const Emissive&>(m);
            g.kind = kGpuMatEmissive;
            Set4(g.data, e.Emission().x, e.Emission().y, e.Emission().z);
            break;
        }
    }
    return g;
}

} // namespace

FourZT::GpuSceneData FourZT::BuildGpuScene(const World& world) {
    GpuSceneData out;
    out.primitives.reserve(world.Get().size());

    // dedupe: many objects usually share one material
    std::unordered_map<const Material*, uint32_t> ids;
    auto MaterialId = [&](const Material* m) -> uint32_t {
        if (!m) throw std::runtime_error("4ZT: object without a material");
        auto [it, inserted] = ids.try_emplace(m, uint32_t(out.materials.size()));
        if (inserted) out.materials.push_back(ToGpuMaterial(*m));
        return it->second;
    };

    for (const auto& obj : world.Get()) {
        GpuPrimitive p{};
        p.material = MaterialId(obj->GetMaterial());

        switch (obj->Kind()) {
            case ObjectKind::Sphere: {
                const auto& s = static_cast<const Sphere&>(*obj);
                p.kind = kGpuPrimSphere;
                Set4(p.a, s.Center().x, s.Center().y, s.Center().z, s.Radius());
                break;
            }
            case ObjectKind::Quad: {
                const auto& q = static_cast<const Quad&>(*obj);
                p.kind = kGpuPrimQuad;
                Set4(p.a, q.Q().x, q.Q().y, q.Q().z);
                Set4(p.b, q.U().x, q.U().y, q.U().z);
                Set4(p.c, q.V().x, q.V().y, q.V().z);
                Set4(p.d, q.Normal().x, q.Normal().y, q.Normal().z);
                Set4(p.e, q.W().x, q.W().y, q.W().z);
                break;
            }
            case ObjectKind::Triangle: {
                const auto& t = static_cast<const Triangle&>(*obj);
                p.kind = kGpuPrimTriangle;
                Set4(p.a, t.P0().x, t.P0().y, t.P0().z);
                Set4(p.b, t.P1().x, t.P1().y, t.P1().z);
                Set4(p.c, t.P2().x, t.P2().y, t.P2().z);
                Set4(p.d, t.Normal().x, t.Normal().y, t.Normal().z);
                Set4(p.e, t.W().x, t.W().y, t.W().z);
                break;
            }
            default:
                continue;   // kinds the GPU doesn't know yet are skipped, like CPU ObjectHit's default
        }
        out.primitives.push_back(p);
    }
    return out;
}