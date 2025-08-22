#pragma once

#include "Core/Scene.hpp"
#include "Core/GameObject.hpp"
#include "Core/SceneManager.hpp"

struct GLFWwindow;

namespace Kiaak
{

    class EditorUI
    {
    public:
        static void Initialize();
        static void InitializeForWindow(GLFWwindow *window);
        static void Shutdown();

        static void BeginFrame();
        static void EndFrame();

        static void RenderProjectBar(Core::SceneManager *sceneManager); // new project UI
                                                                        // (Top bar height internally tracked; other panels offset accordingly)
        static void RenderSceneHierarchy(Core::SceneManager *sceneManager, Core::Scene *&activeScene, Core::GameObject *&selectedObject);
        static void RenderInspector(Core::SceneManager *sceneManager, Core::GameObject *selectedObject);
        static void RenderAssetBrowser();

    private:
        // Asset utilities
        static void RefreshAssetList(bool force = false);
        static const std::vector<std::string> &GetAssetFiles();
    };

}