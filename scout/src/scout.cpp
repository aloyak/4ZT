#include "4ZTScout/scout.h"

#include "4ZTScout/platform/platform.h"
#include "4ZTScout/gui/gui.h"
#include "4ZTScout/render/renderer.h"

#include <chrono>

Scout::App::App() {
    m_context.running = &m_running;
    m_running = true;

    PushLayer<Platform>();
    if (!m_running) return; // platform failed
    PushLayer<Renderer>();
    PushLayer<GUI>();
}

Scout::App::~App() {
    for (auto layer = m_layers.rbegin(); layer != m_layers.rend(); ++layer) {
        (*layer)->OnDetach(m_context);
    }
}

void Scout::App::Run() {
    using clock = std::chrono::steady_clock;
    auto last = clock::now();

    while (m_running) {
        auto now = clock::now();
        float dt = std::chrono::duration<float>(now - last).count();
        last = now;

        for (auto& layer : m_layers) layer->OnUpdate(dt, m_context);
        if (!m_running) break;
        for (auto& layer : m_layers) layer->OnRender(m_context);
    }
}
