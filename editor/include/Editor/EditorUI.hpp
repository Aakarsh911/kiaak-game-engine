#pragma once

#include "Core/Scene.hpp"
#include "Core/GameObject.hpp"

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

        static void RenderSceneHierarchy(Core::Scene *scene, Core::GameObject *&selectedObject);
        static void RenderInspector(Core::GameObject *selectedObject);
        static void RenderAssetBrowser();
    };

}