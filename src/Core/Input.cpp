#include "Core/Input.hpp"
#include <iostream>
#include <vector>

namespace Kiaak {

// Static member definitions
GLFWwindow* Input::s_window = nullptr;
std::unordered_map<int, KeyState> Input::s_keyStates;
std::unordered_map<int, KeyState> Input::s_mouseStates;
double Input::s_mouseX = 0.0;
double Input::s_mouseY = 0.0;
double Input::s_lastMouseX = 0.0;
double Input::s_lastMouseY = 0.0;
double Input::s_mouseDeltaX = 0.0;
double Input::s_mouseDeltaY = 0.0;

void Input::Initialize(GLFWwindow* window) {
    s_window = window;
    
    // Set up GLFW callbacks
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, CursorPositionCallback);
    
    // Get initial mouse position
    glfwGetCursorPos(window, &s_mouseX, &s_mouseY);
    s_lastMouseX = s_mouseX;
    s_lastMouseY = s_mouseY;
    
    std::cout << "Input system initialized" << std::endl;
}

void Input::Update() {
    // Update mouse delta
    s_mouseDeltaX = s_mouseX - s_lastMouseX;
    s_mouseDeltaY = s_mouseY - s_lastMouseY;
    s_lastMouseX = s_mouseX;
    s_lastMouseY = s_mouseY;
    
    for (auto& [key, state] : s_keyStates) {
        if (state == KeyState::Pressed) {
            state = KeyState::Held;
        }
    }
    
    std::vector<int> keysToErase;
    for (auto& [key, state] : s_keyStates) {
        if (state == KeyState::Released) {
            keysToErase.push_back(key);
        }
    }
    for (int key : keysToErase) {
        s_keyStates.erase(key);
    }
    
    for (auto& [button, state] : s_mouseStates) {
        if (state == KeyState::Pressed) {
            state = KeyState::Held;
        }
    }
    
    std::vector<int> buttonsToErase;
    for (auto& [button, state] : s_mouseStates) {
        if (state == KeyState::Released) {
            buttonsToErase.push_back(button);
        }
    }
    for (int button : buttonsToErase) {
        s_mouseStates.erase(button);
    }
}

bool Input::IsKeyPressed(int key) {
    auto it = s_keyStates.find(key);
    return it != s_keyStates.end() && it->second == KeyState::Pressed;
}

bool Input::IsKeyHeld(int key) {
    auto it = s_keyStates.find(key);
    return it != s_keyStates.end() && (it->second == KeyState::Pressed || it->second == KeyState::Held);
}

bool Input::IsKeyReleased(int key) {
    auto it = s_keyStates.find(key);
    return it != s_keyStates.end() && it->second == KeyState::Released;
}

bool Input::IsMouseButtonPressed(MouseButton button) {
    auto it = s_mouseStates.find(static_cast<int>(button));
    return it != s_mouseStates.end() && it->second == KeyState::Pressed;
}

bool Input::IsMouseButtonHeld(MouseButton button) {
    auto it = s_mouseStates.find(static_cast<int>(button));
    return it != s_mouseStates.end() && (it->second == KeyState::Pressed || it->second == KeyState::Held);
}

bool Input::IsMouseButtonReleased(MouseButton button) {
    auto it = s_mouseStates.find(static_cast<int>(button));
    return it != s_mouseStates.end() && it->second == KeyState::Released;
}

double Input::GetMouseX() {
    return s_mouseX;
}

double Input::GetMouseY() {
    return s_mouseY;
}

void Input::GetMousePosition(double& x, double& y) {
    x = s_mouseX;
    y = s_mouseY;
}

double Input::GetMouseDeltaX() {
    return s_mouseDeltaX;
}

double Input::GetMouseDeltaY() {
    return s_mouseDeltaY;
}

// GLFW Callbacks
void Input::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        s_keyStates[key] = KeyState::Pressed;
    } else if (action == GLFW_RELEASE) {
        s_keyStates[key] = KeyState::Released;
    }
}

void Input::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        s_mouseStates[button] = KeyState::Pressed;
    } else if (action == GLFW_RELEASE) {
        s_mouseStates[button] = KeyState::Released;
    }
}

void Input::CursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    s_mouseX = xpos;
    s_mouseY = ypos;
}

} // namespace Kiaak
