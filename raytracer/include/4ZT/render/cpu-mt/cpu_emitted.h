#pragma once

#include "4ZT/world/material.h"

namespace FourZT {

class Emitter {
public:
    static Color Emitted(const Material& material) {
        if (material.Kind() == MaterialKind::Emissive) {
            const Emissive& emissiveMaterial = static_cast<const Emissive&>(material);
            return emissiveMaterial.Emission();
        }
        return Color(0.0, 0.0, 0.0); 
        
    }
};

} // 4zt