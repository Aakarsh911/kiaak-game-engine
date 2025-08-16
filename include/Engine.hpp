#pragma once

#include "Core/Window.hpp"
#include "Graphics/Renderer.hpp"
#include <memory>

namespace Kiaak {

class Engine {
public:
    Engine();
    ~Engine();

    bool Initialize();
    void Run();
    void Shutdown();

private:
    bool isRunning;
    std::unique_ptr<Window> window;
    std::unique_ptr<Renderer> renderer;
};

} // namespace Kiaak
