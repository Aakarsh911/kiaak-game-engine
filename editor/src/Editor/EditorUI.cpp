#include "Editor/EditorUI.hpp"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "Graphics/SpriteRenderer.hpp"
#include "Core/Camera.hpp"
#include "Core/SceneSerialization.hpp"
#include <GLFW/glfw3.h>
#include <functional>
#include <filesystem>
#include <vector>
#include <string>
#if defined(__APPLE__)
extern "C" const char *Kiaak_ShowOpenFileDialog();
#endif

namespace Kiaak
{

    static GLFWwindow *currentWindow = nullptr;
    static std::vector<std::string> g_assetFiles;
    static std::filesystem::file_time_type g_lastAssetScanTime{};
    static double g_lastAssetRefreshCheck = 0.0; // seconds since start
    static const char *kAssetDir = "assets";

    void EditorUI::RefreshAssetList(bool force)
    {
        namespace fs = std::filesystem;
        if (!force)
        {
            // Basic throttle: only rescan if > 1s since last attempt
            double now = ImGui::GetTime();
            if (now - g_lastAssetRefreshCheck < 1.0)
                return;
            g_lastAssetRefreshCheck = now;
        }
        g_assetFiles.clear();
        try
        {
            if (fs::exists(kAssetDir) && fs::is_directory(kAssetDir))
            {
                for (auto &entry : fs::recursive_directory_iterator(kAssetDir))
                {
                    if (!entry.is_regular_file())
                        continue;
                    auto path = entry.path();
                    // Accept common image extensions
                    auto ext = path.extension().string();
                    for (auto &c : ext)
                        c = (char)tolower(c);
                    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp" || ext == ".tga")
                    {
                        g_assetFiles.push_back(path.generic_string());
                    }
                }
            }
        }
        catch (...)
        {
            // swallow exceptions (e.g., permissions)
        }
        std::sort(g_assetFiles.begin(), g_assetFiles.end());
    }

    const std::vector<std::string> &EditorUI::GetAssetFiles()
    {
        if (g_assetFiles.empty())
            RefreshAssetList(true);
        return g_assetFiles;
    }

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
                        RefreshAssetList();
                        const auto &assets = GetAssetFiles();
                        if (assets.empty())
                        {
                            ImGui::MenuItem("<No image assets found>", nullptr, false, false);
                        }
                        for (auto &path : assets)
                        {
                            if (ImGui::MenuItem(path.c_str()))
                            {
                                if (activeScene)
                                {
                                    auto *go = activeScene->CreateGameObject("Sprite");
                                    go->AddComponent<Graphics::SpriteRenderer>(path.c_str());
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
                std::vector<std::string> scenesPendingDelete; // defer until after tree loop
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
                        bool canDelete = (sceneManager->GetSceneCount() > 1);
                        if (ImGui::MenuItem("Delete Scene", nullptr, false, canDelete))
                        {
                            scenesPendingDelete.push_back(sceneName);
                        }
                        if (!canDelete && ImGui::IsItemHovered())
                            ImGui::SetTooltip("Need at least one scene");
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
                            // Draw tree node (label used for ID only so we can custom-draw name + icon)
                            bool open = ImGui::TreeNodeEx((void *)node, flags, "%s", node->GetName().c_str());
                            // Draw icon overlay (camera or sprite) to the left of text inside item rect
                            ImVec2 itemMin = ImGui::GetItemRectMin();
                            ImVec2 itemMax = ImGui::GetItemRectMax();
                            float iconSize = (itemMax.y - itemMin.y) * 0.55f; // square that fits line height
                            float iconPadY = (itemMax.y - itemMin.y - iconSize) * 0.5f;
                            float iconPadX = 4.0f; // small left padding after arrow
                            // Compute where ImGui placed text start: we approximate by itemMin.x + arrow+frame padding
                            // We'll shift left a bit from text start; simpler: draw at itemMin.x + iconPadX
                            ImVec2 iconMin(itemMin.x + iconPadX, itemMin.y + iconPadY);
                            ImVec2 iconMax(iconMin.x + iconSize, iconMin.y + iconSize);
                            auto *dl = ImGui::GetWindowDrawList();
                            bool hasCamera = node->GetComponent<Core::Camera>() != nullptr;
                            bool hasSprite = node->GetComponent<Graphics::SpriteRenderer>() != nullptr;
                            if (hasCamera || hasSprite)
                            {
                                if (hasCamera)
                                {
                                    // Camera icon: body rectangle + lens circle
                                    ImU32 bodyCol = IM_COL32(90, 160, 255, 255);
                                    ImU32 lensCol = IM_COL32(240, 250, 255, 255);
                                    // Body
                                    dl->AddRectFilled(iconMin, iconMax, bodyCol, 2.0f);
                                    // Lens (circle) inside
                                    float lensRadius = iconSize * 0.28f;
                                    ImVec2 lensCenter(iconMin.x + iconSize * 0.65f, iconMin.y + iconSize * 0.5f);
                                    dl->AddCircleFilled(lensCenter, lensRadius, lensCol, 12);
                                    // Small viewfinder protrusion
                                    ImVec2 vf0(iconMin.x - iconSize * 0.25f, iconMin.y + iconSize * 0.15f);
                                    ImVec2 vf1(iconMin.x, iconMin.y + iconSize * 0.55f);
                                    dl->AddRectFilled(vf0, vf1, bodyCol, 1.5f);
                                }
                                else if (hasSprite)
                                {
                                    // Sprite icon: stacked diamond outline + filled square
                                    ImU32 fillCol = IM_COL32(255, 180, 60, 255);
                                    ImU32 outlineCol = IM_COL32(255, 140, 0, 255);
                                    float cx = (iconMin.x + iconMax.x) * 0.5f;
                                    float cy = (iconMin.y + iconMax.y) * 0.5f;
                                    float r = iconSize * 0.45f;
                                    ImVec2 p0(cx, cy - r);
                                    ImVec2 p1(cx + r, cy);
                                    ImVec2 p2(cx, cy + r);
                                    ImVec2 p3(cx - r, cy);
                                    dl->AddQuadFilled(p0, p1, p2, p3, fillCol);
                                    dl->AddQuad(p0, p1, p2, p3, outlineCol, 1.0f);
                                }
                            }
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
                if (!scenesPendingDelete.empty())
                {
                    for (auto &sn : scenesPendingDelete)
                    {
                        bool wasCurrent = (sceneManager->GetCurrentScene() && sceneManager->GetCurrentScene() == sceneManager->GetScene(sn));
                        if (sceneManager->DeleteScene(sn))
                        {
                            // If we deleted selected object's scene, clear selection
                            if (wasCurrent)
                                selectedObject = nullptr;
                        }
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
                // Editable name at top
                static char nameBuffer[256];
                // Copy current name into buffer each frame if different (avoid losing user edits mid-typing?)
                // Simple approach: if buffer doesn't match selected object's name and item not active, sync.
                if (!ImGui::IsAnyItemActive())
                {
                    std::string currentName = selectedObject->GetName();
                    if (strncmp(nameBuffer, currentName.c_str(), sizeof(nameBuffer)) != 0)
                    {
                        strncpy(nameBuffer, currentName.c_str(), sizeof(nameBuffer) - 1);
                        nameBuffer[sizeof(nameBuffer) - 1] = '\0';
                    }
                }
                ImGui::Text("Name");
                ImGui::SameLine();
                ImGui::PushItemWidth(-1);
                if (ImGui::InputText("##GOName", nameBuffer, sizeof(nameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    std::string newName = nameBuffer;
                    if (!newName.empty() && newName != selectedObject->GetName())
                    {
                        selectedObject->SetName(newName);
                        if (sceneManager)
                        {
                            Core::SceneSerialization::SaveAllScenes(sceneManager, "saved_scenes.txt");
                        }
                    }
                }
                ImGui::PopItemWidth();
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
            static char importSource[512] = ""; // absolute or relative path to existing file
            static char importName[256] = "";   // optional target filename
            static std::string importStatus;
            bool doRefresh = false;

            // Manual refresh button
            if (ImGui::Button("Refresh"))
            {
                RefreshAssetList(true);
            }
            ImGui::SameLine();
            ImGui::TextDisabled("(auto refresh throttled)");

            ImGui::SameLine();
            if (ImGui::Button("Import Asset"))
            {
                ImGui::OpenPopup("ImportAssetPopup");
                importStatus.clear();
            }
            if (ImGui::BeginPopupModal("ImportAssetPopup", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Select a file to copy into assets/.");
                ImGui::InputText("Source Path", importSource, sizeof(importSource));
                ImGui::InputText("Target Name (optional)", importName, sizeof(importName));
                ImGui::SameLine();
                if (ImGui::Button("Browse"))
                {
#if defined(__APPLE__)
                    if (const char *chosen = Kiaak_ShowOpenFileDialog())
                    {
                        strncpy(importSource, chosen, sizeof(importSource) - 1);
                        importSource[sizeof(importSource) - 1] = '\0';
                        if (strlen(importName) == 0)
                        {
                            // prefill target name with filename
                            std::filesystem::path p(chosen);
                            auto fname = p.filename().string();
                            strncpy(importName, fname.c_str(), sizeof(importName) - 1);
                            importName[sizeof(importName) - 1] = '\0';
                        }
                    }
#else
                    importStatus = "Native file dialog not implemented on this platform yet";
#endif
                }
                ImGui::TextDisabled("Supported: .png .jpg .jpeg .bmp .tga");
                if (!importStatus.empty())
                {
                    ImGui::Separator();
                    ImGui::TextWrapped("%s", importStatus.c_str());
                }
                if (ImGui::Button("Import"))
                {
                    namespace fs = std::filesystem;
                    try
                    {
                        fs::path src(importSource);
                        if (!fs::exists(src) || !fs::is_regular_file(src))
                        {
                            importStatus = "Source file not found";
                        }
                        else
                        {
                            auto ext = src.extension().string();
                            for (auto &c : ext)
                                c = (char)tolower(c);
                            if (ext != ".png" && ext != ".jpg" && ext != ".jpeg" && ext != ".bmp" && ext != ".tga")
                            {
                                importStatus = "Unsupported extension";
                            }
                            else
                            {
                                fs::path targetDir(kAssetDir);
                                if (!fs::exists(targetDir))
                                    fs::create_directories(targetDir);
                                fs::path targetName = (strlen(importName) > 0) ? fs::path(importName) : src.filename();
                                // ensure extension present if user removed
                                if (targetName.extension().empty())
                                    targetName += src.extension();
                                fs::path target = targetDir / targetName;
                                fs::copy_file(src, target, fs::copy_options::overwrite_existing);
                                importStatus = std::string("Imported -> ") + target.generic_string();
                                doRefresh = true;
                            }
                        }
                    }
                    catch (const std::exception &e)
                    {
                        importStatus = std::string("Error: ") + e.what();
                    }
                    catch (...)
                    {
                        importStatus = "Unknown error";
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Close"))
                {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
            if (doRefresh)
                RefreshAssetList(true);

            RefreshAssetList(); // background refresh attempt
            const auto &assets = GetAssetFiles();
            ImGui::Separator();
            ImGui::BeginChild("AssetList");
            for (auto &a : assets)
            {
                if (ImGui::Selectable(a.c_str()))
                {
                    // Potential future: preview or drag-drop
                }
            }
            ImGui::EndChild();
        }
        ImGui::End();
    }

}