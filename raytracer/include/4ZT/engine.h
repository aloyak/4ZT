#pragma once

#include <memory>

#include "4ZT/render/render.h"

#include "4ZT/camera.h"
#include "4ZT/world/world.h"

#include "4ZT/output/image.h"
#include "4ZT/output/video.h"

#include "4ZT/settings.h"
#include "4ZT/filesystem.h"

namespace FourZT {

class Engine {
public:
    explicit Engine(const Settings& settings);
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    Image Render(const Camera& camera, const World& world);

private:
    Settings m_settings;
    std::unique_ptr<Renderer> m_renderer;
};

} // 4zt