#include "Core/Window.hpp"
#include <iostream>

namespace Kiaak {

Window::Window(int width, int height, const std::string& title)
    : window(nullptr), width(width), height(height), title(title) {}

Window::~Window() {
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

bool Window::Initialize() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // Set OpenGL version and profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Create window
    window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    return true;
}

void Window::Update() {
    glfwPollEvents();
    glfwSwapBuffers(window);
}

bool Window::ShouldClose() const {
    return glfwWindowShouldClose(window);
}

void Window::Close() {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

} // namespace Kiaak
