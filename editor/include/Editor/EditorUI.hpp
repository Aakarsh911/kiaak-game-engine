#pragma once

#include "Core/Scene.hpp"
#include "Core/GameObject.hpp"
#include "Core/SceneManager.hpp"
#include "Core/Collider2D.hpp"
#include <string>
#include <vector>

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
        static void RenderSceneHierarchy(Core::SceneManager *sceneManager, Core::Scene *&activeScene, Core::GameObject *&selectedObject);
        static void RenderInspector(Core::SceneManager *sceneManager, Core::GameObject *selectedObject);
        static void RenderAssetBrowser();
        static void RenderAnimatorPanel(); // bottom row panel next to assets (summary list of clips & editor window trigger)
        static void ApplyPendingAnimationAssignments(Core::Scene *scene);
        // Tilemap painting selection access
        static int GetActiveTilemapPaintIndex();
        static void SetActiveTilemapPaintIndex(int index);
        static bool IsTilemapPaintMode();
        static void SetTilemapPaintMode(bool v);
    static bool IsTilemapColliderMode();
    static void SetTilemapColliderMode(bool v);

        struct AnimationClipInfo
        {
            std::string name;
            std::string texturePath;
            int hFrames = 0;
            int vFrames = 0;
            int cellWidth = 0;
            int cellHeight = 0;
            std::vector<int> sequence;
            float fps = 12.0f;
            bool autoPlay = true;
            bool dirty = false;
        };
        static const std::vector<AnimationClipInfo> &GetAnimationClips();
        static int GetAssignedClip(Core::GameObject *go);                 // -1 if none
        static void SetAssignedClip(Core::GameObject *go, int clipIndex); // pass -1 to clear

    private:
        static void RefreshAssetList(bool force = false);
        static const std::vector<std::string> &GetAssetFiles();
        static void RenderAnimationSheetEditor();
    };

}