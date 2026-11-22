#pragma once

#include <vector>
#include <memory>
#include <cstdint>

#include "4ZTScout/layer.h"

#include <4ZT/settings.h>

/// NOTE: Scout is the same as a "sandbox" or "editor", just wanted to use a cooler name
namespace Scout {

class Platform;
class Window;
class Input;
class Renderer;
class GUI;

struct Settings {
    FourZT::Settings& engineSettings;

    // placeholders for future scout settings
    uint8_t language = 0;
    uint8_t theme = 0;
};

struct Context {
    Window* window = nullptr;
    Input* input = nullptr;
    Platform* platform = nullptr;
    Renderer* renderer = nullptr;
    GUI* gui = nullptr;
    bool* running = nullptr;
};

class App {
public:
    App();
    ~App();

    bool IsRunning() const { return m_running; }
    void Stop() { m_running = false; }

    void Run();
    
    Context& GetContext() { return m_context; }

    template<typename T, typename... Args>
    T& PushLayer(Args&&... args) {
        auto& l = *m_layers.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
        l.OnAttach(m_context);
        return static_cast<T&>(l);
    }

private:
    bool m_running = false;

    std::vector<std::unique_ptr<Layer>> m_layers;
    Context m_context;
};

} // scout