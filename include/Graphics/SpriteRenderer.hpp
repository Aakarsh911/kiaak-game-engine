#pragma once

#include "Core/Component.hpp"
#include "Graphics/Texture.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/VertexArray.hpp"
#include "Graphics/VertexBuffer.hpp"
#include <glm/glm.hpp>
#include <string>
#include <memory>

namespace Kiaak
{
    namespace Graphics
    {

        /**
         * SpriteRenderer component for rendering 2D sprites
         * This replaces the old Sprite class as a component
         */
        class SpriteRenderer : public Core::Component
        {
        public:
            SpriteRenderer();
            SpriteRenderer(const std::string &texturePath);
            ~SpriteRenderer() override;

            // Texture management
            void SetTexture(const std::string &texturePath);
            void SetTexture(std::shared_ptr<Texture> texture);
            Texture *GetTexture() const { return m_texture.get(); }
            const std::string &GetTexturePath() const { return m_texturePath; }

            // Rendering properties
            void SetColor(const glm::vec4 &color) { m_color = color; }
            void SetColor(float r, float g, float b, float a = 1.0f) { m_color = glm::vec4(r, g, b, a); }
            const glm::vec4 &GetColor() const { return m_color; }

            void SetVisible(bool visible) { m_visible = visible; }
            bool IsVisible() const { return m_visible; }

            // Sprite properties
            void SetSize(const glm::vec2 &size) { m_size = size; }
            void SetSize(float width, float height) { m_size = glm::vec2(width, height); }
            const glm::vec2 &GetSize() const { return m_size; }

            // UV coordinates sub-rectangle (u0,v0,u1,v1) within the texture
            // Updates the underlying quad's UVs so no special shader logic is required.
            void SetUVRect(const glm::vec4 &uvRect);
            const glm::vec4 &GetUVRect() const { return m_uvRect; }

            // Rendering
            void Render();

            // Component interface
            void Start() override;
            void Update(double deltaTime) override {}
            std::string GetTypeName() const override { return "SpriteRenderer"; }

        private:
            // Rendering resources
            std::shared_ptr<Texture> m_texture;
            std::unique_ptr<VertexArray> m_vertexArray;
            std::unique_ptr<VertexBuffer> m_vertexBuffer;

            // Sprite properties
            glm::vec4 m_color = glm::vec4(1.0f);                    // White tint by default
            glm::vec2 m_size = glm::vec2(1.0f);                     // Default size
            glm::vec4 m_uvRect = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); // Full texture
            bool m_visible = true;

            // Static shared resources
            static std::shared_ptr<Shader> s_spriteShader;
            static std::shared_ptr<Texture> s_defaultTexture;
            static int s_rendererCount;

            // Internal methods
            void CreateQuad();
            void UpdateQuadUVs(); // rebuilds quad UVs from m_uvRect (no special shader needed)
            void UpdateQuadSize();
            void InitializeShader();
            void CleanupShader();
            void CreateDefaultTexture();
            void CleanupDefaultTexture();

            // Serialization helper
            std::string m_texturePath; // original path used to load the texture (if any)
        };

    } // namespace Graphics
} // namespace Kiaak
