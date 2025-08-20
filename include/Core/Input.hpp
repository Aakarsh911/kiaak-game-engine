#pragma once

#include <GLFW/glfw3.h>
#include <unordered_map>

namespace Kiaak
{

    enum class KeyState
    {
        Released = 0,
        Pressed = 1,
        Held = 2
    };

    enum class MouseButton
    {
        Left = GLFW_MOUSE_BUTTON_LEFT,
        Right = GLFW_MOUSE_BUTTON_RIGHT,
        Middle = GLFW_MOUSE_BUTTON_MIDDLE
    };

    class Input
    {
    public:
        // Initialize input system with window
        static void Initialize(GLFWwindow *window);

        // Update input state (call each frame)
        static void Update();

        // Keyboard input
        static bool IsKeyPressed(int key);
        static bool IsKeyHeld(int key);
        static bool IsKeyReleased(int key);

        // Mouse input
        static bool IsMouseButtonPressed(MouseButton button);
        static bool IsMouseButtonHeld(MouseButton button);
        static bool IsMouseButtonReleased(MouseButton button);

        // Mouse position
        static double GetMouseX();
        static double GetMouseY();
        static void GetMousePosition(double &x, double &y);

        // Mouse delta (movement since last frame)
        static double GetMouseDeltaX();
        static double GetMouseDeltaY();

        // Scroll wheel
        static double GetScrollX();
        static double GetScrollY();
        static void ResetScrollValues();

    private:
        static GLFWwindow *s_window;
        static std::unordered_map<int, KeyState> s_keyStates;
        static std::unordered_map<int, KeyState> s_mouseStates;

        static double s_mouseX, s_mouseY;
        static double s_lastMouseX, s_lastMouseY;
        static double s_mouseDeltaX, s_mouseDeltaY;
        static double s_scrollX, s_scrollY;

        // GLFW callbacks
        static void KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
        static void MouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
        static void CursorPositionCallback(GLFWwindow *window, double xpos, double ypos);
        static void ScrollCallback(GLFWwindow *window, double xoffset, double yoffset);
    };

} // namespace Kiaak
