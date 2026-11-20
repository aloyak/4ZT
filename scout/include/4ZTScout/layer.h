#pragma once

namespace Scout {

struct Context; 

class Layer {
public:
    virtual ~Layer() = default;

    virtual void OnAttach(Context& context) {}
    virtual void OnDetach(Context& context) {}
    virtual void OnUpdate(float dt, Context& context) {}
    virtual void OnRender(Context& context) {}
};

} // scout