#include "Graphics/Renderer.hpp"
#include <iostream>

namespace Kiaak {

Renderer::Renderer() : targetWindow(nullptr), isInitialized(false) {}

Renderer::~Renderer() {
    Shutdown();
}

bool Renderer::Initialize(const Window& window) {
    targetWindow = &window;
    
    if (!InitializeOpenGL()) {
        std::cerr << "Failed to initialize OpenGL" << std::endl;
        return false;
    }

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    
    // Enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    isInitialized = true;
    return true;
}

bool Renderer::InitializeOpenGL() {
    if (!targetWindow) {
        std::cerr << "No target window set" << std::endl;
        return false;
    }
    
    int width = targetWindow->GetWidth();
    int height = targetWindow->GetHeight();
    glViewport(0, 0, width, height);

    return true;
}

void Renderer::BeginFrame() {
    if (!isInitialized) return;
}

void Renderer::EndFrame() {
    if (!isInitialized) return;
}

void Renderer::Clear(float r, float g, float b, float a) {
    if (!isInitialized) return;
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::Shutdown() {
    if (isInitialized) {
        isInitialized = false;
    }
}

} // namespace Kiaak
