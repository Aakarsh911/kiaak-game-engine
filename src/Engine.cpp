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

    isRunning = true;
    return true;
}

void Engine::Run() {
    while (isRunning && !window->ShouldClose()) {
        // Begin frame
        renderer->BeginFrame();
        
        // Clear screen with dark gray color
        renderer->Clear(0.2f, 0.2f, 0.2f);

        // Main game loop will go here

        // End frame
        renderer->EndFrame();
        
        // Update window
        window->Update();
    }
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
