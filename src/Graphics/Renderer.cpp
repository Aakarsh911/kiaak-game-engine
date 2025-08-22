#include "Graphics/Renderer.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/Texture.hpp"
#include "Graphics/VertexArray.hpp"
#include "Graphics/VertexBuffer.hpp"
#include "Core/Camera.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace Kiaak
{

    Renderer::Renderer() : targetWindow(nullptr), isInitialized(false) {}

    Renderer::~Renderer()
    {
        Shutdown();
    }

    bool Renderer::Initialize(const Window &window)
    {
        targetWindow = &window;

        // Initialize GLAD - this loads OpenGL function pointers
        std::cout << "Initializing GLAD..." << std::endl;
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cerr << "Failed to initialize GLAD" << std::endl;
            return false;
        }
        std::cout << "GLAD initialized successfully!" << std::endl;

        // Print OpenGL information
        std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;

        if (!InitializeOpenGL())
        {
            std::cerr << "Failed to initialize OpenGL" << std::endl;
            return false;
        }

        // Initialize quad renderer
        InitializeQuadRenderer();

        // Enable depth testing
        glEnable(GL_DEPTH_TEST);

        // Enable alpha blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        isInitialized = true;
        return true;
    }

    bool Renderer::InitializeOpenGL()
    {
        if (!targetWindow)
        {
            std::cerr << "No target window set" << std::endl;
            return false;
        }

        int width = targetWindow->GetWidth();
        int height = targetWindow->GetHeight();
        glViewport(0, 0, width, height);

        return true;
    }

    void Renderer::BeginFrame(float r, float g, float b, float a)
    {
        if (!isInitialized)
            return;

        // Clear the screen at the start of each frame
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Renderer::EndFrame()
    {
        if (!isInitialized)
            return;

        // Present the rendered frame
        if (targetWindow)
        {
            glfwSwapBuffers(targetWindow->GetNativeWindow());
        }
    }

    void Renderer::Clear(float r, float g, float b, float a)
    {
        if (!isInitialized)
            return;
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Renderer::Shutdown()
    {
        if (isInitialized)
        {
            CleanupQuadRenderer();
            isInitialized = false;
        }
    }

    void Renderer::DrawQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color)
    {
        if (!isInitialized || !m_quadShader || !m_whiteTexture || !m_quadVAO)
            return;

        m_quadShader->Use();

        // Build model matrix
        glm::mat4 model(1.0f);
        model = glm::translate(model, position);
        model = glm::scale(model, glm::vec3(size, 1.0f));

        // Get view-projection matrix from active camera
        glm::mat4 VP(1.0f);
        if (auto *cam = Core::Camera::GetActive())
        {
            VP = cam->GetViewProjection();
        }
        else
        {
            // Fallback to screen-space ortho
            GLint vp[4];
            glGetIntegerv(GL_VIEWPORT, vp);
            float w = static_cast<float>(vp[2]);
            float h = static_cast<float>(vp[3]);
            VP = glm::ortho(-w * 0.5f, w * 0.5f, -h * 0.5f, h * 0.5f, -1.0f, 1.0f);
        }

        // Set uniforms
        m_quadShader->SetMat4("transform", VP * model);
        m_quadShader->SetVec4("color", color);

        // Bind white texture
        m_whiteTexture->Bind(0);
        m_quadShader->SetInt("ourTexture", 0);

        // Render quad
        m_quadVAO->Bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        m_quadVAO->Unbind();
    }

    void Renderer::InitializeQuadRenderer()
    {
        // Create simple vertex shader source
        const char *vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aTexCoord;
        
        uniform mat4 transform;
        
        out vec2 TexCoord;
        
        void main() {
            gl_Position = transform * vec4(aPos, 0.0, 1.0);
            TexCoord = aTexCoord;
        }
    )";

        // Create simple fragment shader source
        const char *fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        
        in vec2 TexCoord;
        uniform sampler2D ourTexture;
        uniform vec4 color;
        
        void main() {
            FragColor = texture(ourTexture, TexCoord) * color;
        }
    )";

        // Create shader
        try
        {
            m_quadShader = std::make_unique<Kiaak::Shader>();
            if (!m_quadShader->LoadFromString(vertexShaderSource, fragmentShaderSource))
            {
                std::cerr << "Failed to create quad shader" << std::endl;
                m_quadShader.reset();
                return;
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Failed to create quad shader: " << e.what() << std::endl;
            return;
        }

        // Create white texture (1x1 white pixel)
        try
        {
            unsigned char whitePixel[] = {255, 255, 255, 255};
            m_whiteTexture = std::make_unique<Texture>();
            if (!m_whiteTexture->CreateFromData(whitePixel, 1, 1, 4))
            {
                std::cerr << "Failed to create white texture" << std::endl;
                m_whiteTexture.reset();
                return;
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Failed to create white texture: " << e.what() << std::endl;
            return;
        }

        // Create quad geometry
        const float vertices[] = {
            // positions  // texture coords
            -0.5f, -0.5f, 0.0f, 0.0f,
            0.5f, -0.5f, 1.0f, 0.0f,
            0.5f, 0.5f, 1.0f, 1.0f,

            -0.5f, -0.5f, 0.0f, 0.0f,
            0.5f, 0.5f, 1.0f, 1.0f,
            -0.5f, 0.5f, 0.0f, 1.0f};

        try
        {
            m_quadVBO = std::make_unique<Kiaak::VertexBuffer>(vertices, sizeof(vertices));
            m_quadVAO = std::make_unique<Kiaak::VertexArray>();

            m_quadVAO->Bind();
            m_quadVBO->Bind();

            // Position attribute
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);

            // Texture coordinate attribute
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

            m_quadVAO->Unbind();
        }
        catch (const std::exception &e)
        {
            std::cerr << "Failed to create quad geometry: " << e.what() << std::endl;
        }
    }

    void Renderer::CleanupQuadRenderer()
    {
        m_quadVAO.reset();
        m_quadVBO.reset();
        m_whiteTexture.reset();
        m_quadShader.reset();
    }

} // namespace Kiaak
