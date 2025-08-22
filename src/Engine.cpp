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

        // Attempt to restore last opened project
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

        // Project-based load: if a project path is set, load all *.scene files inside its scenes folder.
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
        // Fallback: if still no scenes, create default
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

    void Engine::Run()
    {
        while (isRunning && !window->ShouldClose())
        {
            // Poll OS events at the very start of the frame so newly generated
            // input (mouse presses, moves, scroll) is visible to game/editor logic
            // BEFORE we evaluate Pressed/Held logic and before PostFrame() promotes
            // Pressed -> Held. Previously we polled at the end, causing Pressed to
            // be converted to Held in the same frame it arrived (never observable)
            // which broke one-frame interactions like starting a drag.
            window->Update(); // glfwPollEvents()

            timer->update();
            ProcessInput();

            while (timer->shouldUpdateFixed())
            {
                FixedUpdate(timer->getFixedDeltaTime());
            }

            Update(timer->getDeltaTime());
            Render();
            // Advance input states AFTER logic & rendering so Pressed is visible for one full frame
            Input::PostFrame();
        }
    }

    void Engine::ProcessInput()
    {
        if (Input::IsKeyPressed(GLFW_KEY_ESCAPE))
        {
            isRunning = false;
        }

        // Removed 'E' key toggle; play/pause now handled by top bar button.

        Input::Update();
    }

    void Engine::Update(double deltaTime)
    {
        // Handle editor mode input
        if (editorMode)
        {
            HandleEditorInput(deltaTime);
        }

        // Update the scene (calls Update on all GameObjects)
        if (auto *sc = GetCurrentScene())
        {
            sc->Update(deltaTime);
        }

        HandleSpriteClickDetection();

        // Reset scroll values after all input processing is done
        Input::ResetScrollValues();
    }

    void Engine::FixedUpdate(double fixedDeltaTime)
    {
        if (auto *sc = GetCurrentScene())
        {
            sc->FixedUpdate(fixedDeltaTime);
        }
    }

    void Engine::Render()
    {
        renderer->BeginFrame(0.2f, 0.2f, 0.2f, 1.0f);

        if (auto *sc = GetCurrentScene())
        {
            sc->Render();
        }

        if (editorCore)
        {
            EditorUI::BeginFrame();
            if (editorMode)
            {
                RenderSelectionGizmo();
            }
            editorCore->Render(); // internally hides editor-only panels in play mode
            if (editorMode)
            {
                // Sync engine-side selection (used for gizmo) with editor selection (updated via hierarchy clicks)
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
                        editorSel = nullptr; // scene switched; drop stale selection
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
        // Create editor camera only if not already present to avoid duplicates
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
        // Store the current active scene camera before switching
        activeSceneCamera = Core::Camera::GetActive();

        // Switch to editor camera
        if (editorCamera)
        {
            editorCamera->SetActive();
        }
    }

    void Engine::SwitchToPlayMode()
    {
        // Set designated scene camera active if available
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

        // We get mouse coords in logical window space. Projection & viewport use framebuffer (pixel) size.
        // Scale mouse to framebuffer space before converting to NDC to handle HiDPI / Retina.
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

        // Unproject using inverse VP. Z=0 puts us on the "middle" clip-depth; good enough for 2D at world Z=0.
        const glm::mat4 invVP = glm::inverse(cam->GetViewProjection());
        glm::vec4 world = invVP * glm::vec4(x_ndc, y_ndc, 0.0f, 1.0f);
        if (world.w != 0.0f)
            world /= world.w;

        return glm::vec2(world.x, world.y);
    }

    void Engine::HandleSpriteClickDetection()
    {
        // Skip while dragging gizmo
        if (gizmoDragging)
            return;
        if (!Input::IsMouseButtonHeld(MouseButton::Left))
            return;

        // If ImGui wants the mouse (e.g., clicking in a panel), don't perform world selection
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

        // Debug (optional)
        // std::cout << "Mouse: (" << mouseX << ", " << mouseY << ") -> World: (" << worldPos.x << ", " << worldPos.y << ")" << std::endl;

        // Check all GameObjects with SpriteRenderer components
        auto *currentScene = GetCurrentScene();
        if (!currentScene)
            return;

        // Collect all clicked sprites with their Z values
        struct ClickedSprite
        {
            Core::GameObject *gameObject;
            Graphics::SpriteRenderer *spriteRenderer;
            float zValue;
            glm::vec4 bounds; // minX, minY, maxX, maxY
        };

        std::vector<ClickedSprite> clickedSprites;

        auto allGameObjects = currentScene->GetAllGameObjects();
        for (auto *gameObject : allGameObjects)
        {
            if (!gameObject)
                continue;

            // Check if this GameObject has a SpriteRenderer component
            auto *spriteRenderer = gameObject->GetComponent<Graphics::SpriteRenderer>();
            if (!spriteRenderer || !spriteRenderer->IsVisible())
                continue;

            // Get the sprite's transform
            auto *transform = gameObject->GetTransform();
            if (!transform)
                continue;

            glm::vec3 spritePos = transform->GetPosition();
            glm::vec3 spriteScale = transform->GetScale();
            glm::vec2 spriteSize = spriteRenderer->GetSize();

            // Calculate the sprite's world bounds (assuming center-anchored sprite)
            float halfWidth = (spriteSize.x * spriteScale.x) * 0.5f;
            float halfHeight = (spriteSize.y * spriteScale.y) * 0.5f;

            float minX = spritePos.x - halfWidth;
            float maxX = spritePos.x + halfWidth;
            float minY = spritePos.y - halfHeight;
            float maxY = spritePos.y + halfHeight;

            // Check if the world mouse position is within the sprite bounds
            if (worldPos.x >= minX && worldPos.x <= maxX &&
                worldPos.y >= minY && worldPos.y <= maxY)
            {
                // Add to clicked sprites list
                clickedSprites.push_back({gameObject,
                                          spriteRenderer,
                                          spritePos.z, // Z-value for depth sorting
                                          glm::vec4(minX, minY, maxX, maxY)});
            }
        }

        // If any sprites were clicked, find the topmost one (highest Z-value)
        if (!clickedSprites.empty())
        {
            // Sort by Z-value in descending order (highest Z first = topmost)
            std::sort(clickedSprites.begin(), clickedSprites.end(),
                      [](const ClickedSprite &a, const ClickedSprite &b)
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
        auto *spriteRenderer = selectedGameObject->GetComponent<Graphics::SpriteRenderer>();
        if (!transform || !spriteRenderer)
            return; // gizmo only for sprites (for now)

        // Compute world-space AABB
        const glm::vec2 sz = spriteRenderer->GetSize(); // local quad size in world units
        const glm::vec2 half = 0.5f * sz;
        const glm::mat4 M = transform->GetModelMatrix(); // world transform
        const glm::vec4 w0 = M * glm::vec4(-half.x, -half.y, 0.0f, 1.0f);
        const glm::vec4 w1 = M * glm::vec4(half.x, -half.y, 0.0f, 1.0f);
        const glm::vec4 w2 = M * glm::vec4(half.x, half.y, 0.0f, 1.0f);
        const glm::vec4 w3 = M * glm::vec4(-half.x, half.y, 0.0f, 1.0f);

        float minX = std::min(std::min(w0.x, w1.x), std::min(w2.x, w3.x));
        float maxX = std::max(std::max(w0.x, w1.x), std::max(w2.x, w3.x));
        float minY = std::min(std::min(w0.y, w1.y), std::min(w2.y, w3.y));
        float maxY = std::max(std::max(w0.y, w1.y), std::max(w2.y, w3.y));

        const float width = std::max(0.0f, maxX - minX);
        const float height = std::max(0.0f, maxY - minY);
        const glm::vec2 ctr = {(minX + maxX) * 0.5f, (minY + maxY) * 0.5f};
        const float z = transform->GetPosition().z + 0.02f; // nudge above sprite to avoid z-fighting

        // Thickness ~2px in world units
        float thickness = 0.01f; // fallback
        if (auto *cam = Core::Camera::GetActive(); cam && window)
        {
            const float visibleH = 2.0f * cam->GetOrthographicSize() / std::max(cam->GetZoom(), 0.0001f);
            const float perPixelY = visibleH / static_cast<float>(window->GetHeight());
            thickness = std::max(perPixelY * 2.0f, perPixelY); // ~2px
        }

        const glm::vec4 gizmoColor(1.0f, 0.6f, 0.05f, 1.0f);

        // Outline
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
