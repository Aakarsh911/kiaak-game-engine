#include "Engine.hpp"
#include "Graphics/SpriteRenderer.hpp"
#include "Core/Camera.hpp"
#include "Editor/EditorCore.hpp"
#include "Editor/EditorUI.hpp"
#include "Core/SceneSerialization.hpp"
#include "Core/Project.hpp"
#include "imgui.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <fstream>

namespace Kiaak
{

    Engine *Engine::s_instance = nullptr;

    Engine::Engine() : isRunning(false), editorCamera(nullptr), activeSceneCamera(nullptr), editorMode(true), rightMouseDragging(false), editorCameraInitialPosition(0.0f, 0.0f, 5.0f), editorCameraInitialZoom(1.0f), selectedGameObject(nullptr), editorCore(nullptr)
    {
        s_instance = this;
    }
    Engine::~Engine()
    {
        Shutdown();
    }

    // Initialize engine subsystems and load project/scenes
    bool Engine::Initialize()
    {
        window = std::make_unique<Window>(800, 600, "Kiaak Engine");
        if (!window->Initialize())
        {
            return false;
        }

        renderer = std::make_unique<Renderer>();
        if (!renderer->Initialize(*window))
        {
            return false;
        }

        timer = std::make_unique<Timer>();
        Input::Initialize(window->GetNativeWindow());
        sceneManager = std::make_unique<Core::SceneManager>();

        if (!Core::Project::HasPath())
        {
            std::ifstream projIn("last_project.txt");
            if (projIn.is_open())
            {
                std::string pathLine;
                std::getline(projIn, pathLine);
                if (!pathLine.empty() && std::filesystem::exists(pathLine))
                {
                    Core::Project::SetPath(pathLine);
                    Core::Project::EnsureStructure();
                }
            }
        }

        if (Core::Project::HasPath())
        {
            auto scenesPath = Core::Project::GetScenesPath();
            if (std::filesystem::exists(scenesPath))
            {
                for (auto &entry : std::filesystem::directory_iterator(scenesPath))
                {
                    if (!entry.is_regular_file())
                        continue;
                    auto path = entry.path();
                    if (path.extension() == ".scene")
                    {
                        Core::SceneSerialization::LoadSceneFromFile(sceneManager.get(), path.string());
                    }
                }
            }
        }
        if (sceneManager->GetSceneNames().empty())
        {
            sceneManager->CreateScene("MainScene");
        }
        auto *currentScene = sceneManager->GetCurrentScene();

        CreateEditorCamera();

        editorCore = std::make_unique<Kiaak::EditorCore>();
        if (!editorCore->Initialize(window.get(), sceneManager.get(), renderer.get()))
        {
            return false;
        }

        if (currentScene)
        {
            currentScene->Start();
        }

        SwitchToEditorMode();
        isRunning = true;
        return true;
    }

    // Main loop: input, update, render, and advance input states
    void Engine::Run()
    {
        while (isRunning && !window->ShouldClose())
        {
            window->Update();

            timer->update();
            ProcessInput();

            while (timer->shouldUpdateFixed())
            {
                FixedUpdate(timer->getFixedDeltaTime());
            }

            Update(timer->getDeltaTime());
            Render();
            Input::PostFrame();
        }
    }

    // Handle per-frame input
    void Engine::ProcessInput()
    {
        if (Input::IsKeyPressed(GLFW_KEY_ESCAPE))
        {
            isRunning = false;
        }

        Input::Update();
    }

    void Engine::Update(double deltaTime)
    {
        if (editorMode)
        {
            HandleEditorInput(deltaTime);
        }
        if (auto *sc = GetCurrentScene())
        {
            sc->Update(deltaTime);
        }

        HandleSpriteClickDetection();

        Input::ResetScrollValues();
    }

    // Fixed timestep updates
    void Engine::FixedUpdate(double fixedDeltaTime)
    {
        if (auto *sc = GetCurrentScene())
        {
            sc->FixedUpdate(fixedDeltaTime);
        }
    }

    // Render scene and editor UI
    void Engine::Render()
    {
        renderer->BeginFrame(0.2f, 0.2f, 0.2f, 1.0f);

        if (auto *sc = GetCurrentScene())
        {
            sc->Render(editorMode); // include disabled components when in editor mode
        }

        if (editorCore)
        {
            EditorUI::BeginFrame();
            if (editorMode)
            {
                RenderSelectionGizmo();
            }
            // Draw camera view rects for scene cameras (editor only)
            if (editorMode)
            {
                if (auto *sc = GetCurrentScene())
                {
                    for (auto *go : sc->GetAllGameObjects())
                    {
                        if (!go)
                            continue;
                        auto *cam = go->GetComponent<Core::Camera>();
                        if (!cam)
                            continue;

                        const Core::Transform *t = go->GetTransform();
                        glm::vec3 camPos = t ? t->GetPosition() : glm::vec3(0.0f);

                        float halfH = cam->GetOrthographicSize() / std::max(cam->GetZoom(), 0.0001f);
                        float aspect = 1.0f;
                        if (window)
                        {
                            float w = static_cast<float>(window->GetFramebufferWidth());
                            float h = static_cast<float>(window->GetFramebufferHeight());
                            if (h > 0.0f)
                                aspect = w / h;
                        }
                        float halfW = halfH * aspect;

                        glm::vec4 camColor(1.0f, 1.0f, 1.0f, 0.9f);
                        float z = camPos.z + 0.05f;

                        if (go->GetName() == "EditorCamera")
                            continue;

                        float border = 0.01f;
                        if (auto *vcam = cam)
                        {

                            if (window)
                            {
                                float visibleH = 2.0f * vcam->GetOrthographicSize() / std::max(vcam->GetZoom(), 0.0001f);
                                float perPixelY = visibleH / static_cast<float>(window->GetHeight());
                                border = std::max(perPixelY * 2.0f, perPixelY * 0.5f);
                            }
                        }

                        glm::vec2 ctr(camPos.x, camPos.y);
                        float width = halfW * 2.0f;
                        float height = halfH * 2.0f;

                        const float eps = 1e-5f;
                        if (width > eps && height > eps)
                        {
                            renderer->DrawQuad(glm::vec3(ctr.x, camPos.y + halfH + border * 0.5f, z), glm::vec2(width + border * 2.0f, border), camColor);
                            renderer->DrawQuad(glm::vec3(ctr.x, camPos.y - halfH - border * 0.5f, z), glm::vec2(width + border * 2.0f, border), camColor);
                            renderer->DrawQuad(glm::vec3(camPos.x - halfW - border * 0.5f, ctr.y, z), glm::vec2(border, height), camColor);
                            renderer->DrawQuad(glm::vec3(camPos.x + halfW + border * 0.5f, ctr.y, z), glm::vec2(border, height), camColor);

                            float iconSize = std::max(0.05f, std::min(halfW, halfH) * 0.12f);
                            renderer->DrawQuad(glm::vec3(camPos.x, camPos.y, z + 0.02f), glm::vec2(iconSize, iconSize), camColor);
                        }
                    }
                }
            }
            editorCore->Render();
            if (editorMode)
            {
                // Sync engine selection with editor selection
                Core::GameObject *editorSel = editorCore->GetSelectedObject();
                if (editorSel)
                {
                    auto *sc = GetCurrentScene();
                    bool found = false;
                    if (sc)
                    {
                        for (auto *go : sc->GetAllGameObjects())
                        {
                            if (go == editorSel)
                            {
                                found = true;
                                break;
                            }
                        }
                    }
                    if (!found)
                        editorSel = nullptr;
                }
                if (selectedGameObject != editorSel)
                {
                    selectedGameObject = editorSel;
                }
            }
            EditorUI::EndFrame();
        }

        renderer->EndFrame();
    }

    // void Engine::CreateGameObjectDemo()
    // {
    //     auto *imageObject2 = CreateGameObject("ImageSprite2");
    //     auto *imageRenderer2 = imageObject2->AddComponent<Graphics::SpriteRenderer>("assets/background.png");
    //     (void)imageRenderer2;
    //
    //     imageObject2->GetTransform()->SetPosition(0.0f, 0.0f, -1.0f);
    //     imageObject2->GetTransform()->SetScale(10.0f);
    //
    //     auto *imageObject = CreateGameObject("ImageSprite");
    //     auto *imageRenderer = imageObject->AddComponent<Graphics::SpriteRenderer>("assets/spaceship.png");
    //     (void)imageRenderer;
    //
    //     imageObject->GetTransform()->SetPosition(0.0f, -3.0f, 0.0f);
    //     imageObject->GetTransform()->SetScale(5.0f);
    //
    //     auto *camGO = CreateGameObject("MainCamera");
    //     auto *cam = camGO->AddComponent<Core::Camera>();
    //     camGO->GetTransform()->SetPosition(0.0f, 0.0f, 1.0f);
    //     cam->SetOrthographicSize(10.0f);
    //     cam->SetZoom(1.0f);
    //     cam->SetActive();
    // }

    Core::GameObject *Engine::CreateGameObject(const std::string &name)
    {
        auto *currentScene = GetCurrentScene();
        if (!currentScene)
        {
            return nullptr;
        }
        return currentScene->CreateGameObject(name);
    }

    Core::GameObject *Engine::GetGameObject(const std::string &name)
    {
        auto *currentScene = GetCurrentScene();
        if (!currentScene)
        {
            return nullptr;
        }
        return currentScene->GetGameObject(name);
    }

    Core::GameObject *Engine::GetGameObject(uint32_t id)
    {
        auto *currentScene = GetCurrentScene();
        if (!currentScene)
        {
            return nullptr;
        }
        return currentScene->GetGameObject(id);
    }

    bool Engine::RemoveGameObject(const std::string &name)
    {
        auto *currentScene = GetCurrentScene();
        if (!currentScene)
        {
            return false;
        }
        return currentScene->RemoveGameObject(name);
    }

    bool Engine::RemoveGameObject(uint32_t id)
    {
        auto *currentScene = GetCurrentScene();
        if (!currentScene)
        {
            return false;
        }
        return currentScene->RemoveGameObject(id);
    }

    size_t Engine::GetGameObjectCount() const
    {
        auto *currentScene = GetCurrentScene();
        return currentScene ? currentScene->GetGameObjectCount() : 0;
    }

    void Engine::Shutdown()
    {
        if (isRunning)
        {
            if (sceneManager && Core::Project::HasPath())
            {
                auto scenesPath = Core::Project::GetScenesPath();
                std::filesystem::create_directories(scenesPath);
                for (auto &name : sceneManager->GetSceneNames())
                {
                    auto *sc = sceneManager->GetScene(name);
                    if (!sc)
                        continue;
                    Core::SceneSerialization::SaveSceneToFile(sc, scenesPath + "/" + name + ".scene");
                }
                // Persist project path
                std::ofstream projOut("last_project.txt", std::ios::trunc);
                if (projOut.is_open())
                {
                    projOut << Core::Project::GetPath();
                }
            }
            sceneManager.reset();
            renderer.reset();
            window.reset();
            isRunning = false;
        }
    }

    void Engine::ToggleEditorMode()
    {
        editorMode = !editorMode;
        rightMouseDragging = false;

        if (editorMode)
        {
            SwitchToEditorMode();
        }
        else
        {
            SwitchToPlayMode();
        }
    }

    void Engine::TogglePlayPause()
    {
        ToggleEditorMode();
    }

    void Engine::HandleEditorInput(double deltaTime)
    {
        static bool rightMousePressed = false;
        static double lastMouseX, lastMouseY;

        if (Input::IsKeyPressed(GLFW_KEY_R) || Input::IsKeyHeld(GLFW_KEY_R))
        {
            if (editorCamera)
            {
                auto *editorTransform = editorCamera->GetGameObject()->GetTransform();
                editorTransform->SetPosition(editorCameraInitialPosition.x, editorCameraInitialPosition.y, editorCameraInitialPosition.z);
                editorCamera->SetZoom(editorCameraInitialZoom);
                // Ensure immediate visual reset (without waiting for a drag to invalidate)
                editorCamera->InvalidateView();
            }
        }

        if (Input::IsMouseButtonHeld(MouseButton::Right))
        {
            if (!rightMousePressed)
            {
                rightMousePressed = true;
                Input::GetMousePosition(lastMouseX, lastMouseY);
            }
            else
            {
                double currentMouseX, currentMouseY;
                Input::GetMousePosition(currentMouseX, currentMouseY);
                double mouseDeltaX = currentMouseX - lastMouseX;
                double mouseDeltaY = currentMouseY - lastMouseY;

                auto *editorTransform = editorCamera->GetGameObject()->GetTransform();
                glm::vec3 currentPos = editorTransform->GetPosition();
                float sensitivity = 1.0f;
                currentPos.x -= mouseDeltaX * sensitivity;
                currentPos.y += mouseDeltaY * sensitivity;

                editorTransform->SetPosition(currentPos.x, currentPos.y, currentPos.z);
                // Ensure the camera view matrix is marked dirty if not updated elsewhere this frame
                editorCamera->InvalidateView();
                lastMouseX = currentMouseX;
                lastMouseY = currentMouseY;
            }
        }
        else
        {
            rightMousePressed = false;
        }

        double scrollY = Input::GetScrollY();
        if (scrollY != 0.0)
        {
            float currentZoom = editorCamera->GetZoom();
            float zoomSensitivity = 0.1f;
            float newZoom = currentZoom + (scrollY * zoomSensitivity);
            newZoom = glm::clamp(newZoom, 0.01f, 100.0f);
            editorCamera->SetZoom(newZoom);
        }
    }

    void Engine::CreateEditorCamera()
    {
        // Create or reuse the editor camera GameObject
        if (editorCamera)
            return;
        auto *editorCamGO = CreateGameObject("EditorCamera");
        editorCamera = editorCamGO->AddComponent<Core::Camera>();

        editorCameraInitialPosition = glm::vec3(0.0f, 0.0f, 5.0f);
        editorCameraInitialZoom = 1.0f;

        editorCamGO->GetTransform()->SetPosition(editorCameraInitialPosition.x, editorCameraInitialPosition.y, editorCameraInitialPosition.z);
        editorCamera->SetOrthographicSize(10.0f);
        editorCamera->SetZoom(editorCameraInitialZoom);
    }

    void Engine::SwitchToEditorMode()
    {
        // Remember active scene camera, then activate editor camera
        activeSceneCamera = Core::Camera::GetActive();
        if (editorCamera)
            editorCamera->SetActive();
    }

    void Engine::SwitchToPlayMode()
    {
        // Restore designated scene camera or previously active scene camera
        if (auto *sc = GetCurrentScene())
        {
            if (sc->GetDesignatedCamera())
            {
                sc->GetDesignatedCamera()->SetActive();
                return;
            }
        }
        // Fallback to previously stored activeSceneCamera
        if (activeSceneCamera)
        {
            activeSceneCamera->SetActive();
            return;
        }
        // Last resort: keep current active
    }

    glm::vec2 Engine::ScreenToWorld(double mouseX, double mouseY, Core::Camera *cam) const
    {
        if (!cam)
            return glm::vec2(0.0f);

        // Convert logical mouse coords to framebuffer NDC (accounts for HiDPI)
        const double logicalW = static_cast<double>(window->GetWidth());
        const double logicalH = static_cast<double>(window->GetHeight());
        const double fbW = static_cast<double>(window->GetFramebufferWidth());
        const double fbH = static_cast<double>(window->GetFramebufferHeight());
        if (logicalW <= 0.0 || logicalH <= 0.0 || fbW <= 0.0 || fbH <= 0.0)
            return glm::vec2(0.0f);

        const double scaleX = fbW / logicalW;
        const double scaleY = fbH / logicalH;
        const double mouseX_fb = mouseX * scaleX;
        const double mouseY_fb = mouseY * scaleY;

        const float x_ndc = static_cast<float>((mouseX_fb / fbW) * 2.0 - 1.0);
        const float y_ndc = static_cast<float>(1.0 - (mouseY_fb / fbH) * 2.0); // flip Y (window origin top-left)

        // Unproject using inverse ViewProjection into world XY
        const glm::mat4 invVP = glm::inverse(cam->GetViewProjection());
        glm::vec4 world = invVP * glm::vec4(x_ndc, y_ndc, 0.0f, 1.0f);
        if (world.w != 0.0f)
            world /= world.w;

        return glm::vec2(world.x, world.y);
    }

    void Engine::HandleSpriteClickDetection()
    {
        // Skip if gizmo dragging
        if (gizmoDragging)
            return;
        if (!Input::IsMouseButtonHeld(MouseButton::Left))
            return;

        // Ignore when ImGui captures the mouse
        ImGuiIO &io = ImGui::GetIO();
        if (io.WantCaptureMouse)
            return;

        // Get current mouse position
        double mouseX, mouseY;
        Input::GetMousePosition(mouseX, mouseY);

        // Get the active camera
        auto *cam = Core::Camera::GetActive();
        if (!cam)
            return;

        // Convert mouse position to world coordinates
        glm::vec2 worldPos = ScreenToWorld(mouseX, mouseY, cam);

        // Check all GameObjects with SpriteRenderer components
        auto *currentScene = GetCurrentScene();
        if (!currentScene)
            return;

        // Collect clicked items (sprites & cameras)
        struct ClickedItem
        {
            Core::GameObject *gameObject;
            Graphics::SpriteRenderer *spriteRenderer; // nullptr for cameras
            float zValue;
            glm::vec4 bounds; // minX, minY, maxX, maxY
        };

        std::vector<ClickedItem> clickedSprites;

        auto allGameObjects = currentScene->GetAllGameObjects();
        for (auto *gameObject : allGameObjects)
        {
            if (!gameObject)
                continue;

            // First, check sprite renderer
            auto *spriteRenderer = gameObject->GetComponent<Graphics::SpriteRenderer>();
            if (spriteRenderer && spriteRenderer->IsVisible())
            {
                auto *transform = gameObject->GetTransform();
                if (!transform)
                    continue;
                glm::vec3 spritePos = transform->GetPosition();
                glm::vec3 spriteScale = transform->GetScale();
                glm::vec2 spriteSize = spriteRenderer->GetSize();

                float halfWidth = (spriteSize.x * spriteScale.x) * 0.5f;
                float halfHeight = (spriteSize.y * spriteScale.y) * 0.5f;

                float minX = spritePos.x - halfWidth;
                float maxX = spritePos.x + halfWidth;
                float minY = spritePos.y - halfHeight;
                float maxY = spritePos.y + halfHeight;

                if (worldPos.x >= minX && worldPos.x <= maxX && worldPos.y >= minY && worldPos.y <= maxY)
                {
                    clickedSprites.push_back({gameObject, spriteRenderer, spritePos.z, glm::vec4(minX, minY, maxX, maxY)});
                    continue; // found sprite hit, prefer it over camera on same object
                }
            }

            // Next consider camera components (skip editor camera)
            if (gameObject->GetName() == "EditorCamera")
                continue;
            auto *camComp = gameObject->GetComponent<Core::Camera>();
            if (camComp)
            {
                auto *t = gameObject->GetTransform();
                if (!t)
                    continue;
                glm::vec3 camPos = t->GetPosition();
                float halfH = camComp->GetOrthographicSize() / std::max(camComp->GetZoom(), 0.0001f);
                float aspect = 1.0f;
                if (window)
                {
                    float w = static_cast<float>(window->GetFramebufferWidth());
                    float h = static_cast<float>(window->GetFramebufferHeight());
                    if (h > 0.0f)
                        aspect = w / h;
                }
                float halfW = halfH * aspect;
                float minX = camPos.x - halfW;
                float maxX = camPos.x + halfW;
                float minY = camPos.y - halfH;
                float maxY = camPos.y + halfH;

                if (worldPos.x >= minX && worldPos.x <= maxX && worldPos.y >= minY && worldPos.y <= maxY)
                {
                    clickedSprites.push_back({gameObject, nullptr, camPos.z, glm::vec4(minX, minY, maxX, maxY)});
                }
            }
        }

        // If any sprites were clicked, find the topmost one (highest Z-value)
        if (!clickedSprites.empty())
        {
            // Sort by Z-value in descending order (highest Z first = topmost)
            std::sort(clickedSprites.begin(), clickedSprites.end(),
                      [](const ClickedItem &a, const ClickedItem &b)
                      {
                          return a.zValue > b.zValue;
                      });

            const auto &topSprite = clickedSprites[0];
            selectedGameObject = topSprite.gameObject;
            if (editorCore)
            {
                editorCore->SetSelectedObject(selectedGameObject);
            }
            std::cout << "Selected: " << selectedGameObject->GetName() << std::endl;
        }
        else
        {
            selectedGameObject = nullptr;
            if (editorCore)
            {
                editorCore->SetSelectedObject(nullptr);
            }
        }
    }

    void Engine::RenderSelectionGizmo()
    {
        if (!editorMode || !selectedGameObject || !renderer)
            return;

        auto *transform = selectedGameObject->GetTransform();
        if (!transform)
            return;

        // Two supported selection shapes: SpriteRenderer AABB or Camera view rect
        auto *spriteRenderer = selectedGameObject->GetComponent<Graphics::SpriteRenderer>();
        float minX, maxX, minY, maxY;
        if (spriteRenderer)
        {
            // Compute world-space AABB from sprite size and transform
            const glm::vec2 sz = spriteRenderer->GetSize(); // local quad size in world units
            const glm::vec2 half = 0.5f * sz;
            const glm::mat4 M = transform->GetModelMatrix(); // world transform
            const glm::vec4 w0 = M * glm::vec4(-half.x, -half.y, 0.0f, 1.0f);
            const glm::vec4 w1 = M * glm::vec4(half.x, -half.y, 0.0f, 1.0f);
            const glm::vec4 w2 = M * glm::vec4(half.x, half.y, 0.0f, 1.0f);
            const glm::vec4 w3 = M * glm::vec4(-half.x, half.y, 0.0f, 1.0f);

            minX = std::min(std::min(w0.x, w1.x), std::min(w2.x, w3.x));
            maxX = std::max(std::max(w0.x, w1.x), std::max(w2.x, w3.x));
            minY = std::min(std::min(w0.y, w1.y), std::min(w2.y, w3.y));
            maxY = std::max(std::max(w0.y, w1.y), std::max(w2.y, w3.y));
        }
        else
        {
            // If selected object is a camera, compute its view rectangle
            auto *camComp = selectedGameObject->GetComponent<Core::Camera>();
            if (!camComp)
                return; // nothing selectable for gizmo

            glm::vec3 pos = transform->GetPosition();
            float halfH = camComp->GetOrthographicSize() / std::max(camComp->GetZoom(), 0.0001f);
            float aspect = 1.0f;
            if (window)
            {
                float w = static_cast<float>(window->GetFramebufferWidth());
                float h = static_cast<float>(window->GetFramebufferHeight());
                if (h > 0.0f)
                    aspect = w / h;
            }
            float halfW = halfH * aspect;

            minX = pos.x - halfW;
            maxX = pos.x + halfW;
            minY = pos.y - halfH;
            maxY = pos.y + halfH;
        }

        const float width = std::max(0.0f, maxX - minX);
        const float height = std::max(0.0f, maxY - minY);
        const glm::vec2 ctr = {(minX + maxX) * 0.5f, (minY + maxY) * 0.5f};
        const float z = transform->GetPosition().z + 0.02f; // nudge above sprite to avoid z-fighting

        // Thickness ~2px in world units
        float thickness = 0.01f;
        if (auto *cam = Core::Camera::GetActive(); cam && window)
        {
            const float visibleH = 2.0f * cam->GetOrthographicSize() / std::max(cam->GetZoom(), 0.0001f);
            const float perPixelY = visibleH / static_cast<float>(window->GetHeight());
            thickness = std::max(perPixelY * 2.0f, perPixelY); // ~2px
        }

        const glm::vec4 gizmoColor(1.0f, 0.6f, 0.05f, 1.0f);

        // Draw outline
        // Top
        renderer->DrawQuad(glm::vec3(ctr.x, maxY + thickness * 0.5f, z),
                           glm::vec2(width + thickness * 2.0f, thickness),
                           gizmoColor);
        // Bottom
        renderer->DrawQuad(glm::vec3(ctr.x, minY - thickness * 0.5f, z),
                           glm::vec2(width + thickness * 2.0f, thickness),
                           gizmoColor);
        // Left
        renderer->DrawQuad(glm::vec3(minX - thickness * 0.5f, ctr.y, z),
                           glm::vec2(thickness, height),
                           gizmoColor);
        // Right
        renderer->DrawQuad(glm::vec3(maxX + thickness * 0.5f, ctr.y, z),
                           glm::vec2(thickness, height),
                           gizmoColor);

        // Corner handles
        const float handle = thickness * 3.0f;
        renderer->DrawQuad(glm::vec3(minX, minY, z), glm::vec2(handle, handle), gizmoColor);
        renderer->DrawQuad(glm::vec3(maxX, minY, z), glm::vec2(handle, handle), gizmoColor);
        renderer->DrawQuad(glm::vec3(maxX, maxY, z), glm::vec2(handle, handle), gizmoColor);
        renderer->DrawQuad(glm::vec3(minX, maxY, z), glm::vec2(handle, handle), gizmoColor);

        // Drag translate / click outside to deselect
        auto *cam = Core::Camera::GetActive();
        if (!cam)
            return;
        double mx, my;
        Input::GetMousePosition(mx, my);
        glm::vec2 world = ScreenToWorld(mx, my, cam);
        bool inside = (world.x >= minX && world.x <= maxX && world.y >= minY && world.y <= maxY);
        bool leftPressed = Input::IsMouseButtonPressed(MouseButton::Left);
        bool leftHeld = Input::IsMouseButtonHeld(MouseButton::Left);
        bool leftReleased = Input::IsMouseButtonReleased(MouseButton::Left);
        if (!gizmoDragging && leftPressed && !inside && !ImGui::GetIO().WantCaptureMouse)
        {
            // Clicked outside: deselect
            selectedGameObject = nullptr;
            if (editorCore)
                editorCore->SetSelectedObject(nullptr);
            return; // stop drawing interaction this frame
        }
        if (!gizmoDragging && leftPressed && inside && !ImGui::GetIO().WantCaptureMouse)
        {
            gizmoDragging = true;
            gizmoDragStartWorld = world;
            gizmoOriginalPos = transform->GetPosition();
        }
        if (gizmoDragging)
        {
            if (leftHeld)
            {
                glm::vec2 delta = world - gizmoDragStartWorld;
                transform->SetPosition(gizmoOriginalPos + glm::vec3(delta.x, delta.y, 0.0f));
            }
            if (leftReleased)
            {
                gizmoDragging = false;
                if (sceneManager && Core::Project::HasPath())
                {
                    if (auto *sc = GetCurrentScene())
                    {
                        for (auto &nm : sceneManager->GetSceneNames())
                        {
                            if (sceneManager->GetScene(nm) == sc)
                            {
                                Core::SceneSerialization::SaveSceneToFile(sc, Core::Project::GetScenesPath() + "/" + nm + ".scene");
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

} // namespace Kiaak
