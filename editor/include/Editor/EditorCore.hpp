#pragma once

#include "Core/Scene.hpp"
#include "Core/SceneManager.hpp"
#include "Graphics/Renderer.hpp"
#include "Core/Window.hpp"

namespace Kiaak
{
    namespace Core
    {
        class GameObject;
    }

    class EditorCore
    {
    public:
        EditorCore();
        ~EditorCore();

        bool Initialize(const Window *window, Core::SceneManager *manager, Renderer *renderer);
        // Update current scene pointer (called after scene switch)
        void SetScene(Core::Scene *scene) { m_scene = scene; }
        void Update(double deltaTime);
        void Render();
        void Shutdown();

        void SetSelectedObject(Core::GameObject *obj);
        Core::GameObject *GetSelectedObject() const;

    private:
        const Window *m_window;
        Core::Scene *m_scene;
        Core::SceneManager *m_sceneManager{nullptr};
        Renderer *m_renderer;
        Core::GameObject *m_selectedObject;
    };

} // namespace Kiaak