#include "4ZTScout/platform/window.h"

Scout::Window::~Window() {
    Destroy();
}

Scout::Window::Window() {
}

void Scout::Window::Destroy() {
    if (m_renderer) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }
    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
    m_closed = false;
}

bool Scout::Window::Create(const char* title, int width, int height) {
    Destroy();
    m_window = SDL_CreateWindow(title, width, height, SDL_WINDOW_RESIZABLE);
    if (m_window) {
        m_renderer = SDL_CreateRenderer(m_window, nullptr);
        if (!m_renderer) {
            SDL_DestroyWindow(m_window);
            m_window = nullptr;
        }
    }
    m_closed = m_window == nullptr;
    return m_window != nullptr;
}

void Scout::Window::RequestClose() {
    m_closed = true;
}