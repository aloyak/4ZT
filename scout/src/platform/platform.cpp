#include "4ZTScout/platform/platform.h"
#include "4ZTScout/scout.h"

#include <cstdio>

void Scout::Platform::OnAttach(Context& context) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        std::fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
        if (context.running) 
            *context.running = false;
        
        return;
    }

    if (!m_window.Create("4ZT Scout", 1280, 720)) {
        std::fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
        if (context.running) 
            *context.running = false;
        
        return;
    }

    context.window = &m_window;
    context.input = &m_input;
    context.platform = this;
}

void Scout::Platform::OnDetach(Context& context) {
    m_window.Destroy();
    SDL_Quit();

    context.window = nullptr;
    context.input = nullptr;
    context.platform = nullptr;
}

void Scout::Platform::OnUpdate(float deltaTime, Context& context) {
    (void)deltaTime;

    m_events.clear();
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        m_events.push_back(event);
        m_input.ProcessEvent(event);

        if (event.type == SDL_EVENT_QUIT) {
            m_window.RequestClose();
        } else if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
                   m_window.GetHandle() &&
                   event.window.windowID == SDL_GetWindowID(m_window.GetHandle())) {
            m_window.RequestClose();
        }
    }

    m_input.Update();

    if (!m_window.IsOpen() && context.running) {
        *context.running = false;
    }
}

void Scout::Platform::OnRender(Context& context) {
}