#pragma once

#include "Core/Window.hpp"
#include "Core/Timer.hpp"
#include "Core/Input.hpp"
#include "Core/Scene.hpp"
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

    // Public GameObject management API
    Core::GameObject* CreateGameObject(const std::string& name = "GameObject");
    Core::GameObject* GetGameObject(const std::string& name);
    Core::GameObject* GetGameObject(uint32_t id);
    bool RemoveGameObject(const std::string& name);
    bool RemoveGameObject(uint32_t id);

    // Scene utilities
    size_t GetGameObjectCount() const;

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
    void CreateGameObjectDemo();
};

} // namespace Kiaak
