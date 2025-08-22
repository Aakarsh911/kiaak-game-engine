#include "Editor/EditorUI.hpp"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "Graphics/SpriteRenderer.hpp"
#include "Core/Camera.hpp"
#include "Core/SceneSerialization.hpp"
#include <GLFW/glfw3.h>
#include <functional>

namespace Kiaak
{

    static GLFWwindow *currentWindow = nullptr;

    void EditorUI::Initialize()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui::StyleColorsDark();
    }

    void EditorUI::InitializeForWindow(GLFWwindow *window)
    {
        currentWindow = window;
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330 core");
    }

    void EditorUI::Shutdown()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void EditorUI::BeginFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void EditorUI::EndFrame()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    // Unified hierarchy with scenes + objects
    void EditorUI::RenderSceneHierarchy(Core::SceneManager *sceneManager, Core::Scene *&activeScene, Core::GameObject *&selectedObject)
    {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(320, ImGui::GetIO().DisplaySize.y - 200));
        if (ImGui::Begin("Scene Hierarchy", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
        {
            if (sceneManager)
            {
                // Context menu on empty space inside hierarchy window
                if (ImGui::BeginPopupContextWindow("HierarchyContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
                {
                    if (ImGui::MenuItem("New Scene"))
                    {
                        static int sceneCounter = 1;
                        std::string name = "Scene" + std::to_string(sceneCounter++);
                        if (sceneManager->CreateScene(name))
                        {
                            sceneManager->SwitchToScene(name);
                            activeScene = sceneManager->GetCurrentScene();
                            selectedObject = nullptr;
                            // Auto-save after structural change
                            Core::SceneSerialization::SaveAllScenes(sceneManager, "saved_scenes.txt");
                        }
                    }
                    if (ImGui::MenuItem("Create Camera"))
                    {
                        if (activeScene)
                        {
                            auto *go = activeScene->CreateGameObject("Camera");
                            auto *cam = go->AddComponent<Core::Camera>();
                            cam->SetOrthographicSize(10.0f);
                            cam->SetZoom(1.0f);
                            // Do not globally SetActive() here (editor camera remains active in editor mode)
                            activeScene->SetDesignatedCamera(cam);
                            selectedObject = go;
                            Core::SceneSerialization::SaveAllScenes(sceneManager, "saved_scenes.txt");
                        }
                    }
                    if (ImGui::BeginMenu("Create Sprite"))
                    {
                        static const char *images[] = {"assets/spaceship.png", "assets/background.png"};
                        for (auto path : images)
                        {
                            if (ImGui::MenuItem(path))
                            {
                                if (activeScene)
                                {
                                    auto *go = activeScene->CreateGameObject("Sprite");
                                    go->AddComponent<Graphics::SpriteRenderer>(path);
                                    go->GetTransform()->SetPosition(0, 0, 0);
                                    selectedObject = go;
                                    Core::SceneSerialization::SaveAllScenes(sceneManager, "saved_scenes.txt");
                                }
                            }
                        }
                        ImGui::EndMenu();
                    }
                    ImGui::EndPopup();
                }

                // Collect any deletions requested this frame (defer actual removal until after drawing)
                std::vector<Core::GameObject *> pendingDelete;

                auto sceneNames = sceneManager->GetSceneNames();
                for (auto &sceneName : sceneNames)
                {
                    Core::Scene *scene = sceneManager->GetScene(sceneName);
                    bool isActiveScene = (scene == activeScene);
                    ImGuiTreeNodeFlags sceneFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen;
                    std::string label = std::string(isActiveScene ? "[Active] " : "") + sceneName;
                    bool openScene = ImGui::TreeNodeEx((void *)scene, sceneFlags, "%s", label.c_str());
                    if (ImGui::IsItemClicked())
                    {
                        sceneManager->SwitchToScene(sceneName);
                        activeScene = scene;
                        selectedObject = nullptr;
                    }
                    // Scene level context menu
                    if (ImGui::BeginPopupContextItem())
                    {
                        if (ImGui::MenuItem("Set Active"))
                        {
                            sceneManager->SwitchToScene(sceneName);
                            activeScene = scene;
                            selectedObject = nullptr;
                        }
                        ImGui::EndPopup();
                    }

                    if (openScene && scene)
                    {
                        const auto &all = scene->GetAllGameObjects();
                        std::function<void(Core::GameObject *)> drawNode = [&](Core::GameObject *node)
                        {
                            // Skip editor camera object(s) (any name starting with "EditorCamera")
                            if (node->GetName().rfind("EditorCamera", 0) == 0)
                                return; // do not render this node at all

                            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
                            if (node->GetChildren().empty())
                                flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                            if (selectedObject == node)
                                flags |= ImGuiTreeNodeFlags_Selected;
                            bool open = ImGui::TreeNodeEx((void *)node, flags, "%s", node->GetName().c_str());
                            if (ImGui::IsItemClicked())
                                selectedObject = node;

                            // Per-object context menu
                            if (ImGui::BeginPopupContextItem())
                            {
                                if (ImGui::MenuItem("Delete"))
                                {
                                    // Mark for deferred deletion
                                    pendingDelete.push_back(node);
                                    if (selectedObject == node)
                                        selectedObject = nullptr;
                                    ImGui::EndPopup();
                                    return; // don't recurse into (soon-to-be deleted) children
                                }
                                ImGui::EndPopup();
                            }

                            if (open && !node->GetChildren().empty())
                            {
                                for (auto *child : node->GetChildren())
                                    drawNode(child);
                                ImGui::TreePop();
                            }
                        };
                        for (auto *obj : all)
                            if (obj->GetParent() == nullptr)
                                drawNode(obj);
                        ImGui::TreePop();
                    }
                }

                // Execute deletions after tree traversal to avoid use-after-free during ImGui draw.
                if (!pendingDelete.empty() && activeScene)
                {
                    for (auto *go : pendingDelete)
                    {
                        activeScene->RemoveGameObject(go->GetName());
                    }
                    Core::SceneSerialization::SaveAllScenes(sceneManager, "saved_scenes.txt");
                }
            }
        }
        ImGui::End();
    }

    void EditorUI::RenderInspector(Core::SceneManager *sceneManager, Core::GameObject *selectedObject)
    {
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 300, 0));
        ImGui::SetNextWindowSize(ImVec2(300, ImGui::GetIO().DisplaySize.y - 200));

        if (ImGui::Begin("Inspector", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
        {
            if (selectedObject)
            {
                ImGui::Text("GameObject Properties");
                ImGui::Separator();

                auto transform = selectedObject->GetTransform();
                if (transform)
                {
                    ImGui::Text("Transform");
                    // Editable Position (X, Y, Z)
                    glm::vec3 pos = transform->GetPosition();
                    if (ImGui::DragFloat3("Position", &pos.x, 0.1f))
                    {
                        transform->SetPosition(pos);
                    }

                    // Editable Rotation (degrees). For 2D we mainly care about Z, but expose all 3.
                    glm::vec3 rot = transform->GetRotation();
                    if (ImGui::DragFloat3("Rotation", &rot.x, 0.5f))
                    {
                        transform->SetRotation(rot);
                    }

                    // Editable Scale (uniform or per-axis). Ensure scale not zero to avoid degenerate matrix.
                    glm::vec3 scl = transform->GetScale();
                    if (ImGui::DragFloat3("Scale", &scl.x, 0.05f, 0.0001f, 1000.0f))
                    {
                        // Prevent negative zero issues; optionally clamp extremely small values.
                        for (int i = 0; i < 3; ++i)
                        {
                            if (scl[i] == 0.0f)
                                scl[i] = 0.0001f;
                        }
                        transform->SetScale(scl);
                    }

                    ImGui::Separator();
                }

                auto sprite = selectedObject->GetComponent<Graphics::SpriteRenderer>();
                if (sprite)
                {
                    ImGui::Separator();
                    ImGui::Text("Sprite Renderer");
                    ImGui::Text("Has texture: %s", sprite->GetTexture() ? "Yes" : "No");
                }

                // Camera component UI
                if (auto *cam = selectedObject->GetComponent<Core::Camera>())
                {
                    ImGui::Separator();
                    ImGui::Text("Camera");
                    float ortho = cam->GetOrthographicSize();
                    if (ImGui::DragFloat("Ortho Size", &ortho, 0.1f, 0.01f, 1000.0f))
                    {
                        cam->SetOrthographicSize(ortho);
                    }
                    float zoom = cam->GetZoom();
                    if (ImGui::DragFloat("Zoom", &zoom, 0.01f, 0.01f, 100.0f))
                    {
                        cam->SetZoom(zoom);
                    }
                    if (sceneManager)
                    {
                        // Find owning scene
                        Core::Scene *owningScene = nullptr;
                        auto names = sceneManager->GetSceneNames();
                        for (auto &nm : names)
                        {
                            auto *sc = sceneManager->GetScene(nm);
                            if (!sc)
                                continue;
                            auto objs = sc->GetAllGameObjects();
                            for (auto *go : objs)
                            {
                                if (go == selectedObject)
                                {
                                    owningScene = sc;
                                    break;
                                }
                            }
                            if (owningScene)
                                break;
                        }
                        if (owningScene)
                        {
                            bool isDesignated = (owningScene->GetDesignatedCamera() == cam);
                            bool checkbox = isDesignated;
                            if (ImGui::Checkbox("Designated Scene Camera", &checkbox))
                            {
                                if (checkbox)
                                {
                                    // Assign this camera, remove previous implicitly
                                    owningScene->SetDesignatedCamera(cam);
                                }
                                else if (isDesignated)
                                {
                                    // Unassign
                                    owningScene->SetDesignatedCamera(nullptr);
                                }
                                Core::SceneSerialization::SaveAllScenes(sceneManager, "saved_scenes.txt");
                            }
                        }
                    }
                }
            }
            else
            {
                ImGui::Text("No object selected");
            }
        }
        ImGui::End();
    }

    void EditorUI::RenderAssetBrowser()
    {
        ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetIO().DisplaySize.y - 200));
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 200));
        if (ImGui::Begin("Asset Browser", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
        {
            ImGui::Text("Assets will be shown here");
        }
        ImGui::End();
    }

}