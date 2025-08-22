#pragma once

#include "Core/Window.hpp"
#include "Core/Timer.hpp"
#include "Core/Input.hpp"
#include "Core/Scene.hpp"
#include "Core/SceneManager.hpp"
#include "Graphics/Renderer.hpp"
#include "Editor/EditorCore.hpp"
#include <memory>
#include <glm/glm.hpp>

namespace Kiaak
{

    namespace Core
    {
        class Camera;
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
        // Scene management now handled by SceneManager (multiple scenes)
        std::unique_ptr<Core::SceneManager> sceneManager;

        // Convenience accessor for current scene (nullptr if none)
        Core::Scene *GetCurrentScene() const { return sceneManager ? sceneManager->GetCurrentScene() : nullptr; }

        // Camera references for editor
        Core::Camera *editorCamera;
        Core::Camera *activeSceneCamera;

        // Editor mode
        bool editorMode;
        bool rightMouseDragging;

        // Editor camera initial state for reset
        glm::vec3 editorCameraInitialPosition;
        float editorCameraInitialZoom;

        // Selection system
        Core::GameObject *selectedGameObject;

        // Editor system
        std::unique_ptr<Kiaak::EditorCore> editorCore;

        // Game loop functions
        void ProcessInput();
        void Update(double deltaTime);
        void FixedUpdate(double fixedDeltaTime);
        void Render();

        // Demo creation (can be removed later when we have scene loading)
        // void CreateGameObjectDemo(); // disabled

        // Editor mode
        void ToggleEditorMode();
        void HandleEditorInput(double deltaTime);
        void CreateEditorCamera();
        void SwitchToEditorMode();
        void SwitchToPlayMode();

        // Utility functions
        glm::vec2 ScreenToWorld(double mouseX, double mouseY, Core::Camera *cam) const;
        void HandleSpriteClickDetection();
        void RenderSelectionGizmo();
    };

} // namespace Kiaak
