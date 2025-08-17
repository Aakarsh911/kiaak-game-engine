#include "Graphics/Sprite.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

namespace Kiaak {

// Static members
std::unique_ptr<Shader> Sprite::s_spriteShader = nullptr;
int Sprite::s_spriteCount = 0;

Sprite::Sprite(const std::string& imagePath) 
    : m_position(0.0f, 0.0f), m_scale(1.0f, 1.0f), m_rotation(0.0f), m_visible(true) {
    
    std::cout << "Creating sprite from: " << imagePath << std::endl;
    
    // Load texture
    m_texture = std::make_unique<Texture>();
    if (!m_texture->LoadFromFile(imagePath)) {
        std::cerr << "Failed to load sprite texture: " << imagePath << std::endl;
    }
    
    // Create quad geometry
    CreateQuad();
    
    // Initialize shader if this is the first sprite
    if (s_spriteCount == 0) {
        InitializeShader();
    }
    s_spriteCount++;
}

Sprite::~Sprite() {
    s_spriteCount--;
    // If this was the last sprite, clean up shared shader
    if (s_spriteCount == 0) {
        s_spriteShader.reset();
    }
}

void Sprite::SetPosition(float x, float y) {
    m_position = glm::vec2(x, y);
}

void Sprite::SetScale(float scale) {
    m_scale = glm::vec2(scale, scale);
}

void Sprite::SetScale(float scaleX, float scaleY) {
    m_scale = glm::vec2(scaleX, scaleY);
}

void Sprite::SetRotation(float angleDegrees) {
    m_rotation = angleDegrees;
}

void Sprite::Draw() {
    if (!m_visible || !s_spriteShader || !m_texture) {
        return;
    }
    
    // Use sprite shader
    s_spriteShader->Use();
    
    // Calculate transform matrix
    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, glm::vec3(m_position, 0.0f));
    transform = glm::rotate(transform, glm::radians(m_rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    transform = glm::scale(transform, glm::vec3(m_scale, 1.0f));
    
    // Set transform uniform
    s_spriteShader->SetMat4("transform", transform);
    
    // Bind texture
    m_texture->Bind(0);
    s_spriteShader->SetInt("ourTexture", 0);
    
    // Draw quad
    m_vertexArray->Bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Sprite::CreateQuad() {
    // Simple quad vertices: position + texture coordinates
    // Centered around origin for easy transforms
    float vertices[] = {
        // Position        // Texture Coords
        -0.5f,  0.5f,      0.0f, 1.0f,   // Top left
        -0.5f, -0.5f,      0.0f, 0.0f,   // Bottom left  
         0.5f, -0.5f,      1.0f, 0.0f,   // Bottom right
         
        -0.5f,  0.5f,      0.0f, 1.0f,   // Top left (second triangle)
         0.5f, -0.5f,      1.0f, 0.0f,   // Bottom right
         0.5f,  0.5f,      1.0f, 1.0f    // Top right
    };
    
    // Create vertex buffer and array
    m_vertexBuffer = std::make_unique<VertexBuffer>(vertices, sizeof(vertices));
    m_vertexArray = std::make_unique<VertexArray>();
    
    m_vertexArray->Bind();
    m_vertexBuffer->Bind();
    
    // Position attribute
    m_vertexArray->AddAttribute(0, 2, GL_FLOAT, false, 4 * sizeof(float), (void*)0);
    m_vertexArray->EnableAttribute(0);
    
    // Texture coordinate attribute
    m_vertexArray->AddAttribute(1, 2, GL_FLOAT, false, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    m_vertexArray->EnableAttribute(1);
}

void Sprite::InitializeShader() {
    std::string vertexSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aTexCoord;
        
        uniform mat4 transform;
        
        out vec2 texCoord;
        
        void main() {
            gl_Position = transform * vec4(aPos, 0.0, 1.0);
            texCoord = aTexCoord;
        }
    )";
    
    std::string fragmentSource = R"(
        #version 330 core
        out vec4 FragColor;
        
        in vec2 texCoord;
        uniform sampler2D ourTexture;
        
        void main() {
            FragColor = texture(ourTexture, texCoord);
        }
    )";
    
    s_spriteShader = std::make_unique<Shader>();
    if (!s_spriteShader->LoadFromString(vertexSource, fragmentSource)) {
        std::cerr << "Failed to create sprite shader!" << std::endl;
    } else {
        std::cout << "Sprite shader system initialized!" << std::endl;
    }
}

} // namespace Kiaak
