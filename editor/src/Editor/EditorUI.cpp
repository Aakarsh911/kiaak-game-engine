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
                    ImVec2 pos = ImVec2(transform->GetPosition().x, transform->GetPosition().y);
                    ImGui::Text("Position: (%.2f, %.2f)", pos.x, pos.y);

                    ImVec2 scale = ImVec2(transform->GetScale().x, transform->GetScale().y);
                    ImGui::Text("Scale: (%.2f, %.2f)", scale.x, scale.y);

                    ImGui::Text("Rotation: %.2f", transform->GetRotation().z);
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