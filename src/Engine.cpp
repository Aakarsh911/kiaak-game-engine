#include "Engine.hpp"
#include "Graphics/SpriteRenderer.hpp"
#include <iostream>

namespace Kiaak {

Engine::Engine() : isRunning(false) {}

Engine::~Engine() {
    Shutdown();
}

bool Engine::Initialize() {
    std::cout << "Initializing Kiaak Engine..." << std::endl;

    // Create and initialize core components
    window = std::make_unique<Window>(800, 600, "Kiaak Engine");
    if (!window->Initialize()) {
        return false;
    }

    renderer = std::make_unique<Renderer>();
    if (!renderer->Initialize(*window)) {
        return false;
    }

    timer = std::make_unique<Timer>();

    Input::Initialize(window->GetNativeWindow());

    currentScene = std::make_unique<Core::Scene>();

    // Create demo GameObjects
    CreateGameObjectDemo();
    
    // Start the scene
    if (currentScene) {
        currentScene->Start();
    }

    isRunning = true;
    std::cout << "Kiaak Engine initialized successfully!" << std::endl;
    return true;
}

void Engine::Run() {
    std::cout << "Starting main game loop..." << std::endl;
    
    while (isRunning && !window->ShouldClose()) {
        timer->update(); 
        ProcessInput();
        
        // Fixed timestep updates (physics, etc.)
        while (timer->shouldUpdateFixed()) {
            FixedUpdate(timer->getFixedDeltaTime());
        }
        
        // Variable timestep update (rendering, animations)
        Update(timer->getDeltaTime());

        Render();
        window->Update();
    }
    
    std::cout << "Game loop ended." << std::endl;
}

void Engine::ProcessInput() {
    // Handle ESC key to close engine
    if (Input::IsKeyPressed(GLFW_KEY_ESCAPE)) {
        std::cout << "ESC pressed - shutting down engine" << std::endl;
        isRunning = false;
    }


    Input::Update();
}

void Engine::Update(double deltaTime) {
    // Update the scene (calls Update on all GameObjects)
    if (currentScene) {
        currentScene->Update(deltaTime);
    }
    
    // Debug output every second
    static double timeAccumulator = 0.0;
    timeAccumulator += deltaTime;
    if (timeAccumulator >= 1.0) {
        std::cout << "FPS: " << (1.0 / deltaTime) << " | Delta: " << deltaTime * 1000.0 << "ms" << std::endl;
        timeAccumulator = 0.0;
    }
}

void Engine::FixedUpdate(double fixedDeltaTime) {
    // Update the scene at fixed timestep (calls FixedUpdate on all GameObjects)
    if (currentScene) {
        currentScene->FixedUpdate(fixedDeltaTime);
    }
}

void Engine::Render() {
    // Begin frame
    renderer->BeginFrame();
    
    // Clear screen with bright blue color to see window boundaries
    renderer->Clear(0.2f, 0.4f, 0.8f);

    // Render the scene (includes both legacy sprites and GameObjects)
    if (currentScene) {
        currentScene->Render();
    }

    // End frame
    renderer->EndFrame();
}

void Engine::CreateGameObjectDemo() {
    std::cout << "Creating high-level sprite demo..." << std::endl;
    
    auto imageObject = CreateGameObject("ImageSprite");
    auto imageRenderer = imageObject->AddComponent<Graphics::SpriteRenderer>("assets/background.png");
    
    imageObject->GetTransform()->SetPosition(0.0f, 0.0f, 0.0f);
    imageObject->GetTransform()->SetScale(1.0f);
}

// GameObject API implementation
Core::GameObject* Engine::CreateGameObject(const std::string& name) {
    if (!currentScene) {
        std::cout << "Error: No scene available for GameObject creation" << std::endl;
        return nullptr;
    }
    return currentScene->CreateGameObject(name);
}

Core::GameObject* Engine::GetGameObject(const std::string& name) {
    if (!currentScene) {
        return nullptr;
    }
    return currentScene->GetGameObject(name);
}

Core::GameObject* Engine::GetGameObject(uint32_t id) {
    if (!currentScene) {
        return nullptr;
    }
    return currentScene->GetGameObject(id);
}

bool Engine::RemoveGameObject(const std::string& name) {
    if (!currentScene) {
        return false;
    }
    return currentScene->RemoveGameObject(name);
}

bool Engine::RemoveGameObject(uint32_t id) {
    if (!currentScene) {
        return false;
    }
    return currentScene->RemoveGameObject(id);
}

size_t Engine::GetGameObjectCount() const {
    return currentScene ? currentScene->GetGameObjectCount() : 0;
}

void Engine::Shutdown() {
    if (isRunning) {
        std::cout << "Shutting down Kiaak Engine..." << std::endl;
        
        // Clear scene first
        currentScene.reset();
        
        // Then shutdown core systems
        renderer.reset();
        window.reset();
        isRunning = false;
    }
}

} // namespace Kiaak
