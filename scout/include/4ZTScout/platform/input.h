#pragma once

#include <SDL3/SDL.h>

#include <array>
#include <cstdint>

namespace Scout {

class Input {
public:
    void Update();
    void ProcessEvent(const SDL_Event& event);

    bool IsKeyDown(SDL_Scancode key) const;
    bool IsKeyPressed(SDL_Scancode key) const;
    bool IsKeyReleased(SDL_Scancode key) const;

    bool IsMouseDown(uint8_t button) const;
    bool IsMousePressed(uint8_t button) const;
    bool IsMouseReleased(uint8_t button) const;

    float GetMouseX() const { return m_mouseX; }
    float GetMouseY() const { return m_mouseY; }

private:
    std::array<bool, SDL_SCANCODE_COUNT> m_previousKeys{};
    std::array<bool, SDL_SCANCODE_COUNT> m_currentKeys{};
    std::uint32_t m_previousMouseButtons = 0;
    std::uint32_t m_currentMouseButtons = 0;
    
    float m_mouseX = 0.0f;
    float m_mouseY = 0.0f;
};

} // scout
