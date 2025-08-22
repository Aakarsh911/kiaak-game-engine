#include "Core/Window.hpp"
#include <glad/glad.h>
#include <iostream>

namespace Kiaak
{

    Window::Window(int width, int height, const std::string &title)
        : window(nullptr), width(width), height(height), framebufferWidth(width), framebufferHeight(height), title(title) {}

    Window::~Window()
    {
        if (window)
        {
            glfwDestroyWindow(window);
            window = nullptr;
        }
        glfwTerminate();
    }

    bool Window::Initialize()
    {
        if (!glfwInit())
        {
            std::cerr << "Failed to initialize GLFW\n";
            return false;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if (!window)
        {
            std::cerr << "Failed to create GLFW window\n";
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cerr << "Failed to initialize GLAD\n";
            return false;
        }

        // Query initial logical + framebuffer size
        int winW = 0, winH = 0;
        glfwGetWindowSize(window, &winW, &winH);
        width = winW;
        height = winH;
        glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
        glViewport(0, 0, framebufferWidth, framebufferHeight);

        glfwSetWindowUserPointer(window, this);

        // Window size callback (logical size) - this matches cursor coordinate space
        glfwSetWindowSizeCallback(window, [](GLFWwindow *win, int w, int h)
                                  {
        if (auto* self = static_cast<Window*>(glfwGetWindowUserPointer(win))) {
            self->width = w;
            self->height = h;
        } });

        // Framebuffer size callback (pixel size) - update viewport here
        glfwSetFramebufferSizeCallback(window, [](GLFWwindow *win, int fbw, int fbh)
                                       {
        glViewport(0, 0, fbw, fbh);
        if (auto* self = static_cast<Window*>(glfwGetWindowUserPointer(win))) {
            self->framebufferWidth = fbw;
            self->framebufferHeight = fbh;
        } });

        glfwSwapInterval(1); // vsync

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        return true;
    }

    void Window::Update()
    {
        // Only handle events, renderer handles buffer swapping
        glfwPollEvents();
    }

    bool Window::ShouldClose() const
    {
        return glfwWindowShouldClose(window);
    }

    void Window::Close()
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

} // namespace Kiaak
