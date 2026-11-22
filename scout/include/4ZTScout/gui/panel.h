#pragma once

#include <imgui.h>

#include <string>
#include <utility>

#include "4ZTScout/scout.h"

namespace Scout {

class Panel {
public:
    explicit Panel(std::string name)
        : m_name(std::move(name)) {}

    virtual ~Panel() = default;

    const std::string& GetName() const { return m_name; }
    bool& Open() { return m_open; }

    void Render(Context& context) {
        if (!m_open) return;
        if (ImGui::Begin(m_name.c_str(), &m_open))
            OnRender(context);
        ImGui::End();
    }

protected:
    virtual void OnRender(Context& context) = 0;

private:
    std::string m_name;
    bool m_open = true;
};

} // namespace Scout
