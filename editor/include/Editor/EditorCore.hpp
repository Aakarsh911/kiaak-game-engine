#pragma once

#include "Core/Scene.hpp"
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

        bool Initialize(const Window *window, Core::Scene *scene, Renderer *renderer);
        void Update(double deltaTime);
        void Render();
        void Shutdown();

        void SetSelectedObject(Core::GameObject *obj);
        Core::GameObject *GetSelectedObject() const;

    private:
        const Window *m_window;
        Core::Scene *m_scene;
        Renderer *m_renderer;
        Core::GameObject *m_selectedObject;
    };

} // namespace Kiaak