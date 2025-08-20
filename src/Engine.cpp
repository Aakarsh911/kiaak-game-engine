// Engine.cpp
#include "Engine.hpp"
#include "Graphics/SpriteRenderer.hpp"
#include "Core/Camera.hpp" // ← added
#include <glm/glm.hpp>
#include <iostream>

namespace Kiaak
{

    Engine::Engine() : isRunning(false), firstCamera(nullptr), secondCamera(nullptr), editorCamera(nullptr), activeSceneCamera(nullptr), editorMode(true), rightMouseDragging(false), editorCameraInitialPosition(0.0f, 0.0f, 5.0f), editorCameraInitialZoom(1.5f) {}
    Engine::~Engine()
    {
        Shutdown();
    }

    bool Engine::Initialize()
    {
        std::cout << "Initializing Kiaak Engine..." << std::endl;

        // Create and initialize core components
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

        // Create demo GameObjects
        CreateGameObjectDemo();

        // Create editor camera (separate from scene cameras)
        CreateEditorCamera();

        // Start the scene
        if (currentScene)
        {
            currentScene->Start();
        }

        // Switch to editor mode by default (Unity-like behavior)
        SwitchToEditorMode();

        isRunning = true;
        std::cout << "Kiaak Engine initialized successfully!" << std::endl;
        return true;
    }

    void Engine::Run()
    {
        std::cout << "Starting main game loop..." << std::endl;

        while (isRunning && !window->ShouldClose())
        {
            timer->update();
            ProcessInput();

            // Fixed timestep updates (physics, etc.)
            while (timer->shouldUpdateFixed())
            {
                FixedUpdate(timer->getFixedDeltaTime());
            }

            // Variable timestep update (rendering, animations)
            Update(timer->getDeltaTime());

            Render();
            window->Update();
        }

        std::cout << "Game loop ended." << std::endl;
    }

    void Engine::ProcessInput()
    {
        // Handle ESC key to close engine
        if (Input::IsKeyPressed(GLFW_KEY_ESCAPE))
        {
            isRunning = false;
        }

        // Handle editor mode toggle with 'E' key
        if (Input::IsKeyPressed(GLFW_KEY_E))
        {
            ToggleEditorMode();
        }

        // Handle camera switching with '2' key
        if (Input::IsKeyPressed(GLFW_KEY_2))
        {
            SwitchToSecondCamera();
        }

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
        if (currentScene)
        {
            currentScene->Update(deltaTime);
        }

        // Debug output every second
        static double timeAccumulator = 0.0;
        timeAccumulator += deltaTime;
        if (timeAccumulator >= 1.0)
        {
            std::cout << "FPS: " << (1.0 / deltaTime) << " | Delta: " << deltaTime * 1000.0 << "ms";
            if (editorMode)
            {
                std::cout << " | EDITOR MODE";
            }
            std::cout << std::endl;
            timeAccumulator = 0.0;
        }

        // Reset scroll values after all input processing is done
        Input::ResetScrollValues();
    }

    void Engine::FixedUpdate(double fixedDeltaTime)
    {
        // Update the scene at fixed timestep (calls FixedUpdate on all GameObjects)
        if (currentScene)
        {
            currentScene->FixedUpdate(fixedDeltaTime);
        }
    }

    void Engine::Render()
    {
        // Always use dark gray background (Unity-like)
        renderer->BeginFrame(0.2f, 0.2f, 0.2f, 1.0f);

        // Render the scene (includes GameObjects with SpriteRenderer)
        if (currentScene)
        {
            currentScene->Render();
        }

        // End frame (presents/swaps buffers)
        renderer->EndFrame();
    }

    void Engine::CreateGameObjectDemo()
    {
        std::cout << "Creating high-level sprite demo..." << std::endl;

        // First background sprite
        auto *imageObject = CreateGameObject("ImageSprite");
        auto *imageRenderer = imageObject->AddComponent<Graphics::SpriteRenderer>("assets/spaceship.png");
        (void)imageRenderer; // not used after creation

        imageObject->GetTransform()->SetPosition(0.0f, -100.0f, 0.0f);
        imageObject->GetTransform()->SetScale(1.0f);

        // First camera that looks at the background (in XY plane at Z=0)
        auto *camGO = CreateGameObject("MainCamera");
        auto *cam = camGO->AddComponent<Core::Camera>();
        camGO->GetTransform()->SetPosition(0.0f, 0.0f, 1.0f); // in front, looking toward -Z
        cam->SetZoom(1.0f);

        // Store reference to first camera
        firstCamera = cam;
        activeSceneCamera = cam; // Set this as the default scene camera

        // Second sprite at a different position
        auto *imageObject2 = CreateGameObject("ImageSprite2");
        auto *imageRenderer2 = imageObject2->AddComponent<Graphics::SpriteRenderer>("assets/background.png");
        (void)imageRenderer2; // not used after creation

        imageObject2->GetTransform()->SetPosition(0.0f, 200.0f, 0.0f); // Different position
        imageObject2->GetTransform()->SetScale(5.0f);
    }

    void Engine::SwitchToSecondCamera()
    {
        if (secondCamera)
        {
            // Set the second camera as the active camera
            secondCamera->SetActive();
            std::cout << "Switched to second camera!" << std::endl;
        }
        else
        {
            std::cout << "Second camera not available!" << std::endl;
        }
    }

    // GameObject API implementation
    Core::GameObject *Engine::CreateGameObject(const std::string &name)
    {
        if (!currentScene)
        {
            std::cout << "Error: No scene available for GameObject creation" << std::endl;
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
            std::cout << "Shutting down Kiaak Engine..." << std::endl;

            // Clear scene first
            currentScene.reset();

            // Then shutdown core systems
            renderer.reset();
            window.reset();
            isRunning = false;
        }
    }

    void Engine::ToggleEditorMode()
    {
        editorMode = !editorMode;
        rightMouseDragging = false; // Reset dragging state when toggling

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

        // Reset camera to initial position when 'R' is pressed
        if (Input::IsKeyPressed(GLFW_KEY_R) || Input::IsKeyHeld(GLFW_KEY_R))
        {
            if (editorCamera)
            {
                auto *editorTransform = editorCamera->GetGameObject()->GetTransform();
                editorTransform->SetPosition(editorCameraInitialPosition.x, editorCameraInitialPosition.y, editorCameraInitialPosition.z);
                editorCamera->SetZoom(editorCameraInitialZoom);
                std::cout << "Editor camera reset to initial position (" << editorCameraInitialPosition.x << ", " << editorCameraInitialPosition.y << ", " << editorCameraInitialPosition.z << ") with zoom " << editorCameraInitialZoom << std::endl;
            }
            else
            {
                std::cout << "ERROR: Editor camera is null!" << std::endl;
            }
        }

        // Handle camera movement with right-click + drag
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

                // Move camera opposite to mouse movement for natural feel
                auto *editorTransform = editorCamera->GetGameObject()->GetTransform();
                glm::vec3 currentPos = editorTransform->GetPosition();
                float sensitivity = 1.0f;
                currentPos.x -= mouseDeltaX * sensitivity;
                currentPos.y += mouseDeltaY * sensitivity; // Flip Y for screen coordinates

                editorTransform->SetPosition(currentPos.x, currentPos.y, currentPos.z);
                lastMouseX = currentMouseX;
                lastMouseY = currentMouseY;
            }
        }
        else
        {
            rightMousePressed = false;
        }

        // Handle zoom with scroll wheel
        double scrollY = Input::GetScrollY();
        if (scrollY != 0.0)
        {
            std::cout << "Scroll: X=" << Input::GetScrollX() << ", Y=" << scrollY << std::endl;
            std::cout << "Zoom scroll detected: " << scrollY << std::endl;

            float currentZoom = editorCamera->GetZoom();
            std::cout << "Current zoom: " << currentZoom;

            float zoomSensitivity = 0.1f;
            float newZoom = currentZoom + (scrollY * zoomSensitivity);
            newZoom = glm::clamp(newZoom, 0.1f, 10.0f);

            std::cout << " -> New zoom: " << newZoom << std::endl;
            editorCamera->SetZoom(newZoom);
        }
    }

    void Engine::CreateEditorCamera()
    {
        // Create a dedicated editor camera that's separate from the scene
        auto *editorCamGO = CreateGameObject("EditorCamera");
        editorCamera = editorCamGO->AddComponent<Core::Camera>();

        // Set and store initial position and zoom
        editorCameraInitialPosition = glm::vec3(0.0f, 0.0f, 5.0f);
        editorCameraInitialZoom = 1.5f;

        editorCamGO->GetTransform()->SetPosition(editorCameraInitialPosition.x, editorCameraInitialPosition.y, editorCameraInitialPosition.z);
        editorCamera->SetZoom(editorCameraInitialZoom);

        std::cout << "Editor camera created" << std::endl;
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
        // Switch back to the scene camera that was active before editor mode
        if (activeSceneCamera)
        {
            activeSceneCamera->SetActive();
        }
        else if (firstCamera)
        {
            // Fallback to first camera if no stored scene camera
            firstCamera->SetActive();
        }
        else
        {
            std::cout << "PLAY MODE - No scene camera available!" << std::endl;
        }
    }

} // namespace Kiaak
