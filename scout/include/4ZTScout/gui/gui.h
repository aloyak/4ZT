#pragma once

#include "4ZTScout/layer.h"

namespace Scout {

class GUI : public Layer {
    void OnAttach(Context& context) override {}
    void OnDetach(Context& context) override {}
    void OnUpdate(float dt, Context& context) override {}
    void OnRender(Context& context) override {}
};

} // scout