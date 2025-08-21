#include "Engine.hpp"
#include "Graphics/SpriteRenderer.hpp"
#include "Core/Camera.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <algorithm>

namespace Kiaak
{

    Engine::Engine() : isRunning(false), editorCamera(nullptr), activeSceneCamera(nullptr), editorMode(true), rightMouseDragging(false), editorCameraInitialPosition(0.0f, 0.0f, 5.0f), editorCameraInitialZoom(1.0f) {}
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
        currentScene = std::make_unique<Core::Scene>();

        CreateGameObjectDemo();
        CreateEditorCamera();

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
            timer->update();
            ProcessInput();

            while (timer->shouldUpdateFixed())
            {
                FixedUpdate(timer->getFixedDeltaTime());
            }

            Update(timer->getDeltaTime());
            Render();
            window->Update();
        }
    }

    void Engine::ProcessInput()
    {
        if (Input::IsKeyPressed(GLFW_KEY_ESCAPE))
        {
            isRunning = false;
        }

        if (Input::IsKeyPressed(GLFW_KEY_E))
        {
            ToggleEditorMode();
        }

        Input::Update();
    }

    void Engine::Update(double deltaTime)
    {
        if (editorMode)
        {
            HandleEditorInput(deltaTime);
        }

        HandleSpriteClickDetection();

        if (currentScene)
        {
            currentScene->Update(deltaTime);
        }

        Input::ResetScrollValues();
    }

    void Engine::FixedUpdate(double fixedDeltaTime)
    {
        if (currentScene)
        {
            currentScene->FixedUpdate(fixedDeltaTime);
        }
    }

    void Engine::Render()
    {
        renderer->BeginFrame(0.2f, 0.2f, 0.2f, 1.0f);

        if (currentScene)
        {
            currentScene->Render();
        }

        renderer->EndFrame();
    }

    void Engine::CreateGameObjectDemo()
    {
        auto *imageObject2 = CreateGameObject("ImageSprite2");
        auto *imageRenderer2 = imageObject2->AddComponent<Graphics::SpriteRenderer>("assets/background.png");
        (void)imageRenderer2;

        imageObject2->GetTransform()->SetPosition(0.0f, 0.0f, -1.0f);
        imageObject2->GetTransform()->SetScale(10.0f);

        auto *imageObject = CreateGameObject("ImageSprite");
        auto *imageRenderer = imageObject->AddComponent<Graphics::SpriteRenderer>("assets/spaceship.png");
        (void)imageRenderer;

        imageObject->GetTransform()->SetPosition(0.0f, -3.0f, 0.0f);
        imageObject->GetTransform()->SetScale(5.0f);

        auto *camGO = CreateGameObject("MainCamera");
        auto *cam = camGO->AddComponent<Core::Camera>();
        camGO->GetTransform()->SetPosition(0.0f, 0.0f, 1.0f);
        cam->SetOrthographicSize(10.0f);
        cam->SetZoom(1.0f);
        cam->SetActive();
    }

    Core::GameObject *Engine::CreateGameObject(const std::string &name)
    {
        if (!currentScene)
        {
            return nullptr;
        }
        return currentScene->CreateGameObject(name);
    }

    Core::GameObject *Engine::GetGameObject(const std::string &name)
    {
        if (!currentScene)
        {
            return nullptr;
        }
        return currentScene->GetGameObject(name);
    }

    Core::GameObject *Engine::GetGameObject(uint32_t id)
    {
        if (!currentScene)
        {
            return nullptr;
        }
        return currentScene->GetGameObject(id);
    }

    bool Engine::RemoveGameObject(const std::string &name)
    {
        if (!currentScene)
        {
            return false;
        }
        return currentScene->RemoveGameObject(name);
    }

    bool Engine::RemoveGameObject(uint32_t id)
    {
        if (!currentScene)
        {
            return false;
        }
        return currentScene->RemoveGameObject(id);
    }

    size_t Engine::GetGameObjectCount() const
    {
        return currentScene ? currentScene->GetGameObjectCount() : 0;
    }

    void Engine::Shutdown()
    {
        if (isRunning)
        {
            currentScene.reset();
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
        if (activeSceneCamera)
        {
            activeSceneCamera->SetActive();
        }
        else
        {
            auto *sceneCamera = Core::Camera::GetActive();
            if (sceneCamera)
            {
                sceneCamera->SetActive();
            }
        }
    }

    glm::vec2 Engine::ScreenToWorld(double mouseX, double mouseY, Core::Camera *cam) const
    {
        if (!cam)
            return glm::vec2(0.0f);

        // Convert to NDC: X in [-1,1], Y in [-1,1] with Y up (GL-style)
        const float w = static_cast<float>(window->GetWidth());
        const float h = static_cast<float>(window->GetHeight());
        if (w <= 0.0f || h <= 0.0f)
            return glm::vec2(0.0f);

        const float x_ndc = static_cast<float>((mouseX / w) * 2.0 - 1.0);
        const float y_ndc = static_cast<float>(1.0 - (mouseY / h) * 2.0); // flip Y (window origin top-left)

        // Unproject using inverse VP. Z=0 puts us on the "middle" clip-depth; good enough for 2D at world Z=0.
        const glm::mat4 invVP = glm::inverse(cam->GetViewProjection());
        glm::vec4 world = invVP * glm::vec4(x_ndc, y_ndc, 0.0f, 1.0f);
        if (world.w != 0.0f)
            world /= world.w;

        return glm::vec2(world.x, world.y);
    }

    void Engine::HandleSpriteClickDetection()
    {
        if (!Input::IsMouseButtonHeld(MouseButton::Left))
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

            // Log only the topmost sprite
            const auto &topSprite = clickedSprites[0];
            std::cout << "Clicked on sprite: " << topSprite.gameObject->GetName()
                      << " at world position (" << worldPos.x << ", " << worldPos.y << ")"
                      << " | Z-value: " << topSprite.zValue
                      << " | Sprite bounds: [" << topSprite.bounds.x << ", " << topSprite.bounds.y
                      << "] to [" << topSprite.bounds.z << ", " << topSprite.bounds.w << "]";

            // If multiple sprites were overlapping, mention how many
            if (clickedSprites.size() > 1)
            {
                std::cout << " | (Selected topmost of " << clickedSprites.size() << " overlapping sprites)";
            }
            std::cout << std::endl;
        }
    }

} // namespace Kiaak
