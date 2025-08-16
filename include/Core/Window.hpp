#pragma once

#include <GLFW/glfw3.h>
#include <string>

namespace Kiaak {

class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    bool Initialize();
    void Update();
    bool ShouldClose() const;
    void Close();

    // Getters
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    GLFWwindow* GetNativeWindow() const { return window; }

private:
    GLFWwindow* window;
    int width;
    int height;
    std::string title;
    
    // Prevent copying
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
};

} // namespace Kiaak
