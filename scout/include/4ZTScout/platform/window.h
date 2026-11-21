#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

namespace Scout {

class Window {
public:
    Window();
    ~Window();

    bool Create(const char* title, int width, int height);
    void Destroy();

    SDL_Window* GetHandle() const { return m_window; }
    bool IsOpen() const { return m_window != nullptr && !m_closed; }
    void RequestClose();

private:
    SDL_Window* m_window = nullptr;
    bool m_closed = false;
};

} // scout
