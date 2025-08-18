#include "Graphics/SpriteRenderer.hpp"
#include "Core/GameObject.hpp"
#include "Core/Transform.hpp"
#include <glad/glad.h>
#include <iostream>

namespace Kiaak {
namespace Graphics {

// Static member initialization
std::shared_ptr<Shader> SpriteRenderer::s_spriteShader = nullptr;
std::shared_ptr<Texture> SpriteRenderer::s_defaultTexture = nullptr;
int SpriteRenderer::s_rendererCount = 0;

SpriteRenderer::SpriteRenderer() {
    s_rendererCount++;
    CreateQuad();
    if (!s_defaultTexture) {
        CreateDefaultTexture();
    }
}

SpriteRenderer::SpriteRenderer(const std::string& texturePath) : SpriteRenderer() {
    SetTexture(texturePath);
}

SpriteRenderer::~SpriteRenderer() {
    s_rendererCount--;
    CleanupShader();
    CleanupDefaultTexture();
}

void SpriteRenderer::SetTexture(const std::string& texturePath) {
    try {
        m_texture = std::make_shared<Texture>(texturePath);
        
        // Auto-size to texture dimensions if size is default
        if (m_size == glm::vec2(1.0f)) {
            m_size = glm::vec2(m_texture->GetWidth(), m_texture->GetHeight());
            UpdateQuadSize();
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to load texture: " << texturePath << " - " << e.what() << std::endl;
        m_texture = nullptr;
    }
}

void SpriteRenderer::SetTexture(std::shared_ptr<Texture> texture) {
    m_texture = texture;
    
    if (m_texture && m_size == glm::vec2(1.0f)) {
        m_size = glm::vec2(m_texture->GetWidth(), m_texture->GetHeight());
        UpdateQuadSize();
    }
}

void SpriteRenderer::Start() {
    InitializeShader();
}

void SpriteRenderer::Render() {
    if (!m_visible || !s_spriteShader) {
        return;
    }

    auto* transform = GetGameObject()->GetTransform();
    if (!transform) {
        return;
    }

    // Use the shader
    s_spriteShader->Use();
    
    // Set the model matrix
    glm::mat4 model = transform->GetTransformMatrix();
    s_spriteShader->SetMat4("u_model", model);
    
    // Set color
    s_spriteShader->SetVec4("u_color", m_color);
    
    // Use texture if available, otherwise use default white texture
    auto textureToUse = m_texture ? m_texture : s_defaultTexture;
    if (textureToUse) {
        textureToUse->Bind(0);
        s_spriteShader->SetInt("u_texture", 0);
    }
    
    // Render the quad
    m_vertexArray->Bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    m_vertexArray->Unbind();
}

void SpriteRenderer::CreateQuad() {
    // Create vertex data for a quad (two triangles)
    float vertices[] = {
        // Position     // UV coordinates
        -0.5f, -0.5f,   0.0f, 0.0f,  // Bottom-left
         0.5f, -0.5f,   1.0f, 0.0f,  // Bottom-right
         0.5f,  0.5f,   1.0f, 1.0f,  // Top-right
        
        -0.5f, -0.5f,   0.0f, 0.0f,  // Bottom-left
         0.5f,  0.5f,   1.0f, 1.0f,  // Top-right
        -0.5f,  0.5f,   0.0f, 1.0f   // Top-left
    };
    
    m_vertexBuffer = std::make_unique<VertexBuffer>(vertices, sizeof(vertices));
    m_vertexArray = std::make_unique<VertexArray>();
    
    m_vertexArray->Bind();
    m_vertexBuffer->Bind();
    
    // Position attribute (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    
    // UV coordinate attribute (location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    
    m_vertexArray->Unbind();
}

void SpriteRenderer::UpdateQuadSize() {
    // Update vertex data to match the sprite size
    float halfWidth = m_size.x * 0.5f;
    float halfHeight = m_size.y * 0.5f;
    
    float vertices[] = {
        // Position                    // UV coordinates
        -halfWidth, -halfHeight,      m_uvRect.x, m_uvRect.y,                    // Bottom-left
         halfWidth, -halfHeight,      m_uvRect.z, m_uvRect.y,                    // Bottom-right
         halfWidth,  halfHeight,      m_uvRect.z, m_uvRect.w,                    // Top-right
        
        -halfWidth, -halfHeight,      m_uvRect.x, m_uvRect.y,                    // Bottom-left
         halfWidth,  halfHeight,      m_uvRect.z, m_uvRect.w,                    // Top-right
        -halfWidth,  halfHeight,      m_uvRect.x, m_uvRect.w                     // Top-left
    };
    
    m_vertexBuffer->SetData(vertices, sizeof(vertices));
}

void SpriteRenderer::InitializeShader() {
    if (s_spriteShader == nullptr) {
        try {
            s_spriteShader = std::make_shared<Shader>();
            if (!s_spriteShader->LoadFromFile("assets/shaders/basic.vert", "assets/shaders/basic.frag")) {
                std::cerr << "Failed to load sprite shader files" << std::endl;
                s_spriteShader = nullptr;
            }
        } catch (const std::exception& e) {
            std::cerr << "Failed to create sprite shader: " << e.what() << std::endl;
            s_spriteShader = nullptr;
        }
    }
}

void SpriteRenderer::CleanupShader() {
    if (s_rendererCount == 0) {
        s_spriteShader = nullptr;
    }
}

void SpriteRenderer::CreateDefaultTexture() {
    if (s_defaultTexture == nullptr) {
        try {
            // Create a 1x1 white pixel texture
            unsigned char whitePixel[] = {255, 255, 255, 255}; // RGBA white
            s_defaultTexture = std::make_shared<Texture>();
            
            // Create texture from raw data
            if (!s_defaultTexture->CreateFromData(whitePixel, 1, 1, 4)) {
                std::cerr << "Failed to create default texture from data" << std::endl;
                s_defaultTexture = nullptr;
                return;
            }
        } catch (const std::exception& e) {
            std::cerr << "Failed to create default texture: " << e.what() << std::endl;
            s_defaultTexture = nullptr;
        }
    }
}

void SpriteRenderer::CleanupDefaultTexture() {
    if (s_rendererCount == 0) {
        s_defaultTexture = nullptr;
    }
}

} // namespace Graphics
} // namespace Kiaak
