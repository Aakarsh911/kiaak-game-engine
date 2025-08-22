#include "Editor/EditorCore.hpp"
#include "Editor/EditorUI.hpp"

namespace Kiaak
{

    EditorCore::EditorCore() : m_window(nullptr), m_scene(nullptr), m_renderer(nullptr), m_selectedObject(nullptr)
    {
    }

    EditorCore::~EditorCore()
    {
        EditorUI::Shutdown();
    }

    bool EditorCore::Initialize(const Window *win, Core::SceneManager *manager, Renderer *rend)
    {
        m_window = win;
        m_sceneManager = manager;
        m_scene = manager ? manager->GetCurrentScene() : nullptr;
        m_renderer = rend;

        EditorUI::Initialize();
        EditorUI::InitializeForWindow(m_window->GetNativeWindow());
        return true;
    }

    void EditorCore::Render()
    {
        if (m_sceneManager)
        {
            Core::Scene *sceneRef = m_scene;
            EditorUI::RenderSceneHierarchy(m_sceneManager, sceneRef, m_selectedObject);
            if (sceneRef != m_scene)
                m_scene = sceneRef;
        }
        EditorUI::RenderInspector(m_sceneManager, m_selectedObject);
        EditorUI::RenderAssetBrowser();
    }

    void EditorCore::SetSelectedObject(Core::GameObject *obj)
    {
        m_selectedObject = obj;
    }

    Core::GameObject *EditorCore::GetSelectedObject() const
    {
        return m_selectedObject;
    }

}
