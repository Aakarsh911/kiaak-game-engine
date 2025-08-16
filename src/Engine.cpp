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

    isRunning = true;
    return true;
}

void Engine::Run() {
    while (isRunning && !window->ShouldClose()) {
        // Update window
        window->Update();

        // Main game loop will go here
    }
}

void Engine::Shutdown() {
    if (isRunning) {
        std::cout << "Shutting down Kiaak Engine..." << std::endl;
        window.reset();
        isRunning = false;
    }
}

} // namespace Kiaak
