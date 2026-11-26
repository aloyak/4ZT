#include "4ZT/engine.h"

namespace FourZT {

    Engine::Engine(const Settings& settings) : m_settings(settings) {
        FileSystem::Mount("shaders");

        m_renderer = Renderer::Create(m_settings);
    }

    Image Engine::Render(const Camera& camera, const World& world) {
        return m_renderer->Render(m_settings, camera, world);
    }

    Engine::~Engine() {
        FileSystem::Unmount("shaders");
    }

} // 4zt