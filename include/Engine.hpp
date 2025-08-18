#pragma once

#include "Core/Window.hpp"
#include "Core/Timer.hpp"
#include "Core/Input.hpp"
#include "Core/Scene.hpp"
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

    // Public sprite management API
    Kiaak::Sprite* CreateSprite(const std::string& id, const std::string& texturePath);
    Kiaak::Sprite* GetSprite(const std::string& id);
    bool RemoveSprite(const std::string& id);
    void SetSpriteLayer(const std::string& id, int layer);

    // Scene utilities
    size_t GetSpriteCount() const;
    std::vector<std::string> GetSpriteIds() const;

private:
    // Core systems
    bool isRunning;
    std::unique_ptr<Window> window;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<Timer> timer;
    
    // Scene management
    std::unique_ptr<Core::Scene> currentScene;
    
    // Game loop functions
    void ProcessInput();
    void Update(double deltaTime);
    void FixedUpdate(double fixedDeltaTime);
    void Render();
    
    // Demo creation (can be removed later when we have scene loading)
    void CreateSpriteDemo();
};

} // namespace Kiaak
