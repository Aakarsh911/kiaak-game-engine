#pragma once

#include "Core/Window.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/Texture.hpp"
#include "Graphics/VertexArray.hpp"
#include "Graphics/VertexBuffer.hpp"
#include <glm/glm.hpp>
#include <memory>

namespace Kiaak
{

    class Renderer
    {
    public:
        Renderer();
        ~Renderer();

        bool Initialize(const Window &window);
        void BeginFrame(float r = 0.2f, float g = 0.4f, float b = 0.8f, float a = 1.0f);
        void EndFrame();
        void Clear(float r, float g, float b, float a = 1.0f);
        void Shutdown();

        // Simple quad drawing for gizmos and debug rendering
        void DrawQuad(const glm::vec3 &position, const glm::vec2 &size, const glm::vec4 &color);

    private:
        bool InitializeOpenGL();
        void InitializeQuadRenderer();
        void CleanupQuadRenderer();

        const Window *targetWindow;
        bool isInitialized;

        // Quad rendering resources
        std::unique_ptr<Kiaak::Shader> m_quadShader;
        std::unique_ptr<Texture> m_whiteTexture;
        std::unique_ptr<Kiaak::VertexArray> m_quadVAO;
        std::unique_ptr<Kiaak::VertexBuffer> m_quadVBO;
    };

} // namespace Kiaak
