#include "Editor/EditorCore.hpp"
#include "Editor/EditorUI.hpp"
#include "Engine.hpp"

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
        // Always draw top bar (contains play/pause + project controls)
        if (m_sceneManager)
        {
            EditorUI::RenderProjectBar(m_sceneManager);
        }
        // Only show editor panels when in editor mode (play mode hides hierarchy/inspector/asset browser)
        if (Engine::Get() && Engine::Get()->IsEditorMode())
        {
            bool scriptMode = EditorUI::IsScriptEditorOpen();
            if (m_sceneManager)
            {
                Core::Scene *sceneRef = m_scene;
                EditorUI::RenderSceneHierarchy(m_sceneManager, sceneRef, m_selectedObject);
                if (sceneRef != m_scene)
                    m_scene = sceneRef;
            }
            if (!scriptMode)
            {
                EditorUI::RenderInspector(m_sceneManager, m_selectedObject);
                EditorUI::RenderAssetBrowser();
                EditorUI::RenderAnimatorPanel();
            }
            if (scriptMode)
            {
                EditorUI::RenderScriptEditorOverlay();
            }
        }
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
