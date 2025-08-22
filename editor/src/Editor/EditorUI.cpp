#include "Editor/EditorUI.hpp"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "Graphics/SpriteRenderer.hpp"
#include <GLFW/glfw3.h>

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

    void EditorUI::RenderSceneHierarchy(Core::Scene *scene, Core::GameObject *&selectedObject)
    {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(300, ImGui::GetIO().DisplaySize.y - 200));

        if (ImGui::Begin("Scene Hierarchy", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
        {
            if (scene)
            {
                const auto &gameObjects = scene->GetAllGameObjects();
                for (size_t i = 0; i < gameObjects.size(); ++i)
                {
                    auto obj = gameObjects[i];
                    bool isSelected = (selectedObject == obj);

                    if (ImGui::Selectable(obj->GetName().c_str(), isSelected))
                    {
                        selectedObject = obj;
                    }
                }
            }
        }
        ImGui::End();
    }

    void EditorUI::RenderInspector(Core::GameObject *selectedObject)
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