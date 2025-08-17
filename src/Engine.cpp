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

    // Create high-level sprite demo
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
    
    // Example input handling (you can remove this later)
    if (Input::IsKeyPressed(GLFW_KEY_SPACE)) {
        std::cout << "Space pressed!" << std::endl;
    }
    
    if (Input::IsMouseButtonPressed(MouseButton::Left)) {
        double x, y;
        Input::GetMousePosition(x, y);
        std::cout << "Left mouse clicked at: " << x << ", " << y << std::endl;
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

    // High-level sprite rendering - clean and simple!
    if (demoSprite) {
        demoSprite->Draw();
    }

    // End frame
    renderer->EndFrame();
}

void Engine::CreateSpriteDemo() {
    std::cout << "Creating high-level sprite demo..." << std::endl;
    
    // This is how easy it is for game developers!
    // Just one line to create a sprite from an image
    demoSprite = std::make_unique<Sprite>("assets/image.jpg");
    
    // Easy transforms - no manual matrix math!
    demoSprite->SetPosition(0.0f, 0.0f);  // Center of screen
    demoSprite->SetScale(0.8f);           // Scale down a bit so we can see the whole image
    
    std::cout << "Sprite demo created! No manual OpenGL calls needed!" << std::endl;
}

void Engine::Shutdown() {
    if (isRunning) {
        std::cout << "Shutting down Kiaak Engine..." << std::endl;
        renderer.reset();
        window.reset();
        isRunning = false;
    }
}

} // namespace Kiaak
