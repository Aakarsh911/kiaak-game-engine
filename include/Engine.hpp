#pragma once

#include "Core/Window.hpp"
#include "Core/Timer.hpp"
#include "Core/Input.hpp"
#include "Graphics/Renderer.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/VertexBuffer.hpp"
#include "Graphics/VertexArray.hpp"
#include "Graphics/Texture.hpp"
#include "Graphics/Sprite.hpp"
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
    
    // High-level sprite system
    std::unique_ptr<Sprite> demoSprite;
    
    // Game loop functions
    void ProcessInput();
    void Update(double deltaTime);
    void FixedUpdate(double fixedDeltaTime);
    void Render();
    
    // Sprite system initialization
    void CreateSpriteDemo();
};

} // namespace Kiaak
