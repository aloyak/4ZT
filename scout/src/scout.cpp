#include "4ZTScout/scout.h"
#include "4ZTScout/platform/platform.h"

Scout::App::App() {
    m_context.running = &m_running;
    m_running = true;
    PushLayer<Platform>();
}

Scout::App::~App() {
    for (auto layer = m_layers.rbegin(); layer != m_layers.rend(); ++layer) {
        (*layer)->OnDetach(m_context);
    }
}

void Scout::App::Run() {
    for (auto& layer : m_layers) {
        layer->OnUpdate(0.0f, m_context); 
    }

    for (auto& layer : m_layers) {
        layer->OnRender(m_context);
    }
}
