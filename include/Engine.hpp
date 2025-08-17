#pragma once

#include "Core/Window.hpp"
#include "Core/Timer.hpp"
#include "Core/Input.hpp"
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
    // Core systems
    bool isRunning;
    std::unique_ptr<Window> window;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<Timer> timer;
    
    // Game loop functions
    void ProcessInput();
    void Update(double deltaTime);
    void FixedUpdate(double fixedDeltaTime);
    void Render();
};

} // namespace Kiaak
