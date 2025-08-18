#include "Engine.hpp"
#include <iostream>

namespace Kiaak {

Engine::Engine() : isRunning(false) {}

Engine::~Engine() {
    Shutdown();
}

bool Engine::Initialize() {
    std::cout << "Initializing Kiaak Engine..." << std::endl;
    
    // Create and initialize window
    window = std::make_unique<Window>(800, 600, "Kiaak Engine");
    if (!window->Initialize()) {
        return false;
    }

    // Create and initialize renderer
    renderer = std::make_unique<Renderer>();
    if (!renderer->Initialize(*window)) {
        return false;
    }

    // Create timer
    timer = std::make_unique<Timer>();

    // Initialize input system
    Input::Initialize(window->GetNativeWindow());

    // Create scene for sprite management
    currentScene = std::make_unique<Core::Scene>();

    // Create demo sprites
    CreateSpriteDemo();

    isRunning = true;
    std::cout << "Kiaak Engine initialized successfully!" << std::endl;
    return true;
}

void Engine::Run() {
    std::cout << "Starting main game loop..." << std::endl;
    
    while (isRunning && !window->ShouldClose()) {
        // Update timer
        timer->update(); 
        
        // Process input
        ProcessInput();
        
        // Fixed timestep updates (physics, etc.)
        while (timer->shouldUpdateFixed()) {
            FixedUpdate(timer->getFixedDeltaTime());
        }
        
        // Variable timestep update (rendering, animations)
        Update(timer->getDeltaTime());
        
        // Render frame
        Render();
        
        // Update window
        window->Update();
    }
    
    std::cout << "Game loop ended." << std::endl;
}

void Engine::ProcessInput() {
    // Update input system
    Input::Update();
    
    // Handle ESC key to close engine
    if (Input::IsKeyPressed(GLFW_KEY_ESCAPE)) {
        std::cout << "ESC pressed - shutting down engine" << std::endl;
        isRunning = false;
    }
    
    // Example input handling
    if (Input::IsKeyPressed(GLFW_KEY_SPACE)) {
        std::cout << "✅ Space key pressed!" << std::endl;
    }
    
    if (Input::IsMouseButtonPressed(MouseButton::Left)) {
        double x, y;
        Input::GetMousePosition(x, y);
        std::cout << "✅ Mouse click at: " << x << ", " << y << std::endl;
    }
}

void Engine::Update(double deltaTime) {
    // Update game logic that depends on frame time
    // Examples: animations, smooth movements, camera transitions
    
    // Debug output every second
    static double timeAccumulator = 0.0;
    timeAccumulator += deltaTime;
    if (timeAccumulator >= 1.0) {
        std::cout << "FPS: " << (1.0 / deltaTime) << " | Delta: " << deltaTime * 1000.0 << "ms" << std::endl;
        timeAccumulator = 0.0;
    }
}

void Engine::FixedUpdate(double fixedDeltaTime) {
    // Update systems that need consistent timing
    // Examples: physics, collision detection, game logic
}

void Engine::Render() {
    // Begin frame
    renderer->BeginFrame();
    
    // Clear screen with dark gray color
    renderer->Clear(0.2f, 0.2f, 0.2f);

    // Render all sprites in the scene
    if (currentScene) {
        currentScene->RenderAll();
    }

    // End frame
    renderer->EndFrame();
}

void Engine::CreateSpriteDemo() {
    std::cout << "Creating sprite demo..." << std::endl;
    
    // Background sprite (layer 0 - renders first)
    auto background = CreateSprite("background", "../assets/image.jpg");
    background->SetPosition(0.0f, 0.5f);  // Top
    background->SetScale(0.2f);
    SetSpriteLayer("background", 0);
    
    // Main sprite (layer 1 - renders on top of background)
    auto mainSprite = CreateSprite("main", "../assets/image2.jpg");
    mainSprite->SetPosition(0.0f, 0.0f);  // Center
    mainSprite->SetScale(0.2f);
    SetSpriteLayer("main", 1);
    
    // Foreground sprite (layer 2 - renders on top)
    auto foreground = CreateSprite("foreground", "../assets/image3.jpg");
    foreground->SetPosition(0.0f, -0.5f);  // Bottom
    foreground->SetScale(0.2f);
    SetSpriteLayer("foreground", 2);
    
    std::cout << "Sprite demo created! Total sprites: " << GetSpriteCount() << std::endl;
    std::cout << "Rendering order: background -> main -> foreground" << std::endl;
}

// Public API implementation
Kiaak::Sprite* Engine::CreateSprite(const std::string& id, const std::string& texturePath) {
    if (!currentScene) {
        std::cout << "Error: No scene available for sprite creation" << std::endl;
        return nullptr;
    }
    return currentScene->CreateSprite(id, texturePath);
}

Kiaak::Sprite* Engine::GetSprite(const std::string& id) {
    if (!currentScene) {
        return nullptr;
    }
    return currentScene->GetSprite(id);
}

bool Engine::RemoveSprite(const std::string& id) {
    if (!currentScene) {
        return false;
    }
    return currentScene->RemoveSprite(id);
}

void Engine::SetSpriteLayer(const std::string& id, int layer) {
    if (currentScene) {
        currentScene->SetSpriteLayer(id, layer);
    }
}

size_t Engine::GetSpriteCount() const {
    return currentScene ? currentScene->GetSpriteCount() : 0;
}

std::vector<std::string> Engine::GetSpriteIds() const {
    return currentScene ? currentScene->GetSpriteIds() : std::vector<std::string>();
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
