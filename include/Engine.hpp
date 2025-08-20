#pragma once

#include "Core/Window.hpp"
#include "Core/Timer.hpp"
#include "Core/Input.hpp"
#include "Core/Scene.hpp"
#include "Graphics/Renderer.hpp"
#include <memory>
#include <glm/glm.hpp>

namespace Kiaak
{

    namespace Core
    {
        class Camera; // Forward declaration
    }

    class Engine
    {
    public:
        Engine();
        ~Engine();

        bool Initialize();
        void Run();
        void Shutdown();

        // Public GameObject management API
        Core::GameObject *CreateGameObject(const std::string &name = "GameObject");
        Core::GameObject *GetGameObject(const std::string &name);
        Core::GameObject *GetGameObject(uint32_t id);
        bool RemoveGameObject(const std::string &name);
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

        // Camera references for switching
        Core::Camera *firstCamera;
        Core::Camera *secondCamera;
        Core::Camera *editorCamera;
        Core::Camera *activeSceneCamera;

        // Editor mode
        bool editorMode;
        bool rightMouseDragging;

        // Editor camera initial state for reset
        glm::vec3 editorCameraInitialPosition;
        float editorCameraInitialZoom;

        // Game loop functions
        void ProcessInput();
        void Update(double deltaTime);
        void FixedUpdate(double fixedDeltaTime);
        void Render();

        // Demo creation (can be removed later when we have scene loading)
        void CreateGameObjectDemo();

        // Camera switching
        void SwitchToSecondCamera();

        // Editor mode
        void ToggleEditorMode();
        void HandleEditorInput(double deltaTime);
        void CreateEditorCamera();
        void SwitchToEditorMode();
        void SwitchToPlayMode();
    };

} // namespace Kiaak
