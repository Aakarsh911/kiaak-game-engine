#pragma once

#include "Texture.hpp"
#include "Shader.hpp"
#include "VertexArray.hpp"
#include "VertexBuffer.hpp"
#include <glm/glm.hpp>
#include <string>
#include <memory>

namespace Kiaak {

/**
 * High-level Sprite class for easy 2D rendering
 * Hides all the OpenGL complexity behind a simple interface
 */
class Sprite {
public:
    /**
     * Create sprite from image file
     * @param imagePath Path to image file
     */
    Sprite(const std::string& imagePath);
    
    /**
     * Destructor
     */
    ~Sprite();
    
    // Transform methods - easy to use!
    void SetPosition(float x, float y);
    void SetScale(float scale);
    void SetScale(float scaleX, float scaleY);
    void SetRotation(float angleDegrees);
    
    // Getters
    glm::vec2 GetPosition() const { return m_position; }
    glm::vec2 GetScale() const { return m_scale; }
    float GetRotation() const { return m_rotation; }
    
    // Rendering
    void Draw();
    
    // Properties
    void SetVisible(bool visible) { m_visible = visible; }
    bool IsVisible() const { return m_visible; }
    
private:
    // Transform properties
    glm::vec2 m_position;
    glm::vec2 m_scale;
    float m_rotation;
    bool m_visible;
    
    // Rendering resources (hidden from user!)
    std::unique_ptr<Texture> m_texture;
    std::unique_ptr<VertexArray> m_vertexArray;
    std::unique_ptr<VertexBuffer> m_vertexBuffer;
    
    // Static shared resources
    static std::unique_ptr<Shader> s_spriteShader;
    static int s_spriteCount;
    
    // Internal methods
    void CreateQuad();
    void UpdateTransform();
    void InitializeShader();
};

} // namespace Kiaak
