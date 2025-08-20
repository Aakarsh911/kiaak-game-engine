#include "Graphics/SpriteRenderer.hpp"
#include "Core/GameObject.hpp"
#include "Core/Transform.hpp"
#include "Core/Camera.hpp"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace Kiaak
{
    namespace Graphics
    {

        std::shared_ptr<Shader> SpriteRenderer::s_spriteShader = nullptr;
        std::shared_ptr<Texture> SpriteRenderer::s_defaultTexture = nullptr;
        int SpriteRenderer::s_rendererCount = 0;

        static constexpr float PPU = 100.0f;

        SpriteRenderer::SpriteRenderer()
        {
            s_rendererCount++;
            CreateQuad();
            if (!s_defaultTexture)
                CreateDefaultTexture();
        }

        SpriteRenderer::SpriteRenderer(const std::string &texturePath) : SpriteRenderer()
        {
            SetTexture(texturePath);
        }

        SpriteRenderer::~SpriteRenderer()
        {
            s_rendererCount--;
            CleanupShader();
            CleanupDefaultTexture();
        }

        void SpriteRenderer::SetTexture(const std::string &texturePath)
        {
            try
            {
                m_texture = std::make_shared<Texture>(texturePath);
                if (m_texture && m_size == glm::vec2(1.0f))
                {
                    // Size defaults to texture pixel size
                    m_size = glm::vec2(m_texture->GetWidth(), m_texture->GetHeight());
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "Failed to load texture: " << texturePath << " - " << e.what() << std::endl;
                m_texture = nullptr;
            }
        }

        void SpriteRenderer::SetTexture(std::shared_ptr<Texture> texture)
        {
            m_texture = texture;
            if (m_texture && m_size == glm::vec2(1.0f))
            {
                m_size = glm::vec2(m_texture->GetWidth(), m_texture->GetHeight());
            }
        }

        void SpriteRenderer::Start()
        {
            InitializeShader();
        }

        void SpriteRenderer::Render()
        {
            if (!m_visible || !s_spriteShader)
                return;

            auto *transform = GetGameObject()->GetTransform();
            if (!transform)
                return;

            s_spriteShader->Use();

            // Build model matrix from Transform + explicit sprite size
            glm::mat4 model(1.0f);
            const glm::vec3 pos = transform->GetPosition();
            const glm::vec3 rot = transform->GetRotation();
            const glm::vec3 scale = transform->GetScale();

            model = glm::translate(model, pos);
            model = glm::rotate(model, glm::radians(rot.z), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, glm::vec3(m_size, 1.0f));
            model = glm::scale(model, scale);

            // If we have an active camera, use its VP; otherwise use screen-space ortho
            glm::mat4 VP(1.0f);
            if (auto *cam = Core::Camera::GetActive())
            {
                VP = cam->GetViewProjection();
            }
            else
            {
                GLint vp[4];
                glGetIntegerv(GL_VIEWPORT, vp);
                float w = static_cast<float>(vp[2]);
                float h = static_cast<float>(vp[3]);
                VP = glm::ortho(-w * 0.5f, w * 0.5f, -h * 0.5f, h * 0.5f, -1.0f, 1.0f);
            }

            s_spriteShader->SetMat4("transform", VP * model);

            auto textureToUse = m_texture ? m_texture : s_defaultTexture;
            if (textureToUse)
            {
                textureToUse->Bind(0);
                s_spriteShader->SetInt("ourTexture", 0);
            }

            m_vertexArray->Bind();
            glDrawArrays(GL_TRIANGLES, 0, 6);
            m_vertexArray->Unbind();
        }

        void SpriteRenderer::CreateQuad()
        {
            // unit quad centered at origin
            const float vertices[] = {
                // pos       // uv
                -0.5f, -0.5f, 0.0f, 0.0f,
                0.5f, -0.5f, 1.0f, 0.0f,
                0.5f, 0.5f, 1.0f, 1.0f,

                -0.5f, -0.5f, 0.0f, 0.0f,
                0.5f, 0.5f, 1.0f, 1.0f,
                -0.5f, 0.5f, 0.0f, 1.0f};

            m_vertexBuffer = std::make_unique<VertexBuffer>(vertices, sizeof(vertices));
            m_vertexArray = std::make_unique<VertexArray>();

            m_vertexArray->Bind();
            m_vertexBuffer->Bind();

            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);

            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

            m_vertexArray->Unbind();
        }

        void SpriteRenderer::InitializeShader()
        {
            if (s_spriteShader)
                return;
            try
            {
                s_spriteShader = std::make_shared<Shader>();
                if (!s_spriteShader->LoadFromFile("assets/shaders/basic.vert", "assets/shaders/basic.frag"))
                {
                    std::cerr << "Failed to load sprite shader files\n";
                    s_spriteShader = nullptr;
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "Failed to create sprite shader: " << e.what() << std::endl;
                s_spriteShader = nullptr;
            }
        }

        void SpriteRenderer::CleanupShader()
        {
            if (s_rendererCount == 0)
                s_spriteShader.reset();
        }

        void SpriteRenderer::CreateDefaultTexture()
        {
            if (s_defaultTexture)
                return;
            try
            {
                unsigned char px[] = {255, 255, 255, 255};
                s_defaultTexture = std::make_shared<Texture>();
                if (!s_defaultTexture->CreateFromData(px, 1, 1, 4))
                {
                    std::cerr << "Failed to create default texture\n";
                    s_defaultTexture = nullptr;
                }
            }
            catch (const std::exception &e)
            {
                std::cerr << "Failed to create default texture: " << e.what() << std::endl;
                s_defaultTexture = nullptr;
            }
        }

        void SpriteRenderer::CleanupDefaultTexture()
        {
            if (s_rendererCount == 0)
                s_defaultTexture.reset();
        }

    } // namespace Graphics
} // namespace Kiaak
