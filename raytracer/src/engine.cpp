#include "4ZT/engine.h"

namespace FourZT {

    Engine::Engine(const Settings& settings) : m_settings(settings) {

    }

    Image Engine::Render(const Camera& camera, const World& world) {
        auto renderer = Renderer::Create(m_settings);
        return renderer->Render(m_settings, camera, world);
    }

} // 4zt