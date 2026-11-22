#pragma once

#include "4ZTScout/gui/panel.h"

namespace Scout {

class ConsolePanel : public Panel {
public:
    ConsolePanel() : Panel("Console") {}

protected:
    void OnRender(Context& context) override {
        ImGui::TextDisabled("No messages.");
    }
};

} // namespace Scout
