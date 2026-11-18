#pragma once

#include <utility>
#include <vector>

#include "4ZT/object.h"
#include "4ZT/material.h"

namespace FourZT {
    
class World {
public:
    World() = default;
    ~World() = default;

    void Add(std::unique_ptr<Object> obj) {
        m_objects.push_back(std::move(obj));
    }

    template <class T, class... Args>
    void Add(Args&&... args) {
        m_objects.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    }

    const std::vector<std::unique_ptr<Object>>& Get() const {
        return m_objects;
    }

private:
    // world is made out of a list of objects with the "Object" class
    std::vector<std::unique_ptr<Object>> m_objects;
};

} // 4zt