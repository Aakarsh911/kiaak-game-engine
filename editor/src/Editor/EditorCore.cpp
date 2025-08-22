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

    bool EditorCore::Initialize(const Window *win, Core::Scene *sc, Renderer *rend)
    {
        m_window = win;
        m_scene = sc;
        m_renderer = rend;

        EditorUI::Initialize();
        EditorUI::InitializeForWindow(m_window->GetNativeWindow());
        return true;
    }

    void EditorCore::Render()
    {
        EditorUI::RenderSceneHierarchy(m_scene, m_selectedObject);
        EditorUI::RenderInspector(m_selectedObject);
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
