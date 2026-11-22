#pragma once

#include "4ZTScout/layer.h"

#include "4ZTScout/platform/window.h"
#include "4ZTScout/platform/input.h"

#include <vector>

namespace Scout {

class Platform : public Layer {
public:
    Platform() = default;

    void OnAttach(Context& context) override;
    void OnDetach(Context& context) override;
    void OnUpdate(float dt, Context& context) override;
    void OnRender(Context& context) override;

    Window& GetWindow() { return m_window; }
    Input& GetInput() { return m_input; }

    const std::vector<SDL_Event>& GetEvents() const { return m_events; }

private:
    Window m_window;
    Input m_input;

    std::vector<SDL_Event> m_events;
};

} // scout
