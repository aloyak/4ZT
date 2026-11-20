#include "4ZTScout/platform/input.h"

#include <algorithm>
#include <cstring>

void Scout::Input::Update() {
    m_previousKeys = m_currentKeys;

    int keyCount = 0;
    const bool* keyboardState = SDL_GetKeyboardState(&keyCount);
    if (keyboardState) {
        m_currentKeys.fill(false);
        const std::size_t copyCount = std::min<std::size_t>(
            m_currentKeys.size(), static_cast<std::size_t>(keyCount));
        std::memcpy(m_currentKeys.data(), keyboardState, copyCount);
    } else {
        m_currentKeys.fill(false);
    }

    m_previousMouseButtons = m_currentMouseButtons;
    m_currentMouseButtons = SDL_GetMouseState(&m_mouseX, &m_mouseY);
}

void Scout::Input::ProcessEvent(const SDL_Event&) {
}

bool Scout::Input::IsKeyDown(SDL_Scancode key) const {
    return key >= 0 && key < SDL_SCANCODE_COUNT && m_currentKeys[key];
}

bool Scout::Input::IsKeyPressed(SDL_Scancode key) const {
    return key >= 0 && key < SDL_SCANCODE_COUNT &&
           m_currentKeys[key] && !m_previousKeys[key];
}

bool Scout::Input::IsKeyReleased(SDL_Scancode key) const {
    return key >= 0 && key < SDL_SCANCODE_COUNT &&
           !m_currentKeys[key] && m_previousKeys[key];
}

bool Scout::Input::IsMouseDown(uint8_t button) const {
    return (m_currentMouseButtons & SDL_BUTTON_MASK(button)) != 0;
}

bool Scout::Input::IsMousePressed(uint8_t button) const {
    return (m_currentMouseButtons & SDL_BUTTON_MASK(button)) != 0 &&
           (m_previousMouseButtons & SDL_BUTTON_MASK(button)) == 0;
}

bool Scout::Input::IsMouseReleased(uint8_t button) const {
    return (m_currentMouseButtons & SDL_BUTTON_MASK(button)) == 0 &&
           (m_previousMouseButtons & SDL_BUTTON_MASK(button)) != 0;
}
