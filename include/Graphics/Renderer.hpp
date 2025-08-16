#pragma once

#include "Core/Window.hpp"
#include <memory>

namespace Kiaak {

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool Initialize(const Window& window);
    void BeginFrame();
    void EndFrame();
    void Clear(float r, float g, float b, float a = 1.0f);
    void Shutdown();

private:
    bool InitializeOpenGL();
    
    const Window* targetWindow;
    bool isInitialized;
};

} // namespace Kiaak
