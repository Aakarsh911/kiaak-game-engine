#pragma once

#include <GLFW/glfw3.h>
#include <string>

namespace Kiaak
{

    class Window
    {
    public:
        Window(int width, int height, const std::string &title);
        ~Window();

        bool Initialize();
        void Update();
        bool ShouldClose() const;
        void Close();

        // Getters
        // Logical window size (in screen coordinates - same space as cursor positions)
        int GetWidth() const { return width; }
        int GetHeight() const { return height; }
        // Framebuffer size (in pixels - use for glViewport / rendering)
        int GetFramebufferWidth() const { return framebufferWidth; }
        int GetFramebufferHeight() const { return framebufferHeight; }
        GLFWwindow *GetNativeWindow() const { return window; }

    private:
        GLFWwindow *window;
        int width;             // logical window width
        int height;            // logical window height
        int framebufferWidth;  // pixel width of framebuffer
        int framebufferHeight; // pixel height of framebuffer
        std::string title;

        // Prevent copying
        Window(const Window &) = delete;
        Window &operator=(const Window &) = delete;
    };

} // namespace Kiaak
