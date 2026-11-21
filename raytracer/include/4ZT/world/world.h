#pragma once

#include <utility>
#include <vector>

#include "4ZT/world/object.h"
#include "4ZT/world/composite.h"
#include "4ZT/world/material.h"

#include "4ZT/math/math.h"

namespace FourZT {
    
class World {
public:
    World() = default;
    ~World() = default;

    void Add(std::unique_ptr<Object> obj) {
        m_objects.push_back(std::move(obj));
    }

    void Add(std::unique_ptr<Composite> composite) {
        auto primitives = composite->GetPrimitives();
        for (auto& obj : primitives) {
            m_objects.push_back(std::move(obj));
        }
    }

    // Allows both default objects or composites
    template <class T, class... Args>
    void Add(Args&&... args) {
        if constexpr (std::is_base_of_v<Composite, T>) {
            T composite(std::forward<Args>(args)...);
            auto primitives = composite.GetPrimitives();
            for (auto& obj : primitives) {
                m_objects.push_back(std::move(obj));
            }
        } else {
            m_objects.push_back(std::make_unique<T>(std::forward<Args>(args)...));
        }
    }

    const std::vector<std::unique_ptr<Object>>& Get() const {
        return m_objects;
    }

private:
    // world is made out of a list of objects with the "Object" class
    std::vector<std::unique_ptr<Object>> m_objects;
};

} // 4zt