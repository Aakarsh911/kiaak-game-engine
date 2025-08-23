#include "Graphics/Texture.hpp"
#include <iostream>
#include <unordered_set>

// Define STB_IMAGE_IMPLEMENTATION before including stb_image.h
// This tells stb_image to include the implementation, not just declarations
#define STB_IMAGE_IMPLEMENTATION
#include "../../external/stb/stb_image.h"

// Static members
std::unordered_set<Texture *> Texture::s_allTextures{};
Texture::FilterMode Texture::s_currentFilterMode = Texture::FilterMode::Linear;

Texture::Texture()
    : m_textureID(0), m_width(0), m_height(0), m_channels(0)
{
    s_allTextures.insert(this);
}

Texture::Texture(const std::string &filePath)
    : m_textureID(0), m_width(0), m_height(0), m_channels(0)
{
    s_allTextures.insert(this);
    LoadFromFile(filePath);
}

Texture::~Texture()
{
    s_allTextures.erase(this);
    Cleanup();
}

bool Texture::LoadFromFile(const std::string &filePath)
{
    // Clean up any existing texture
    Cleanup();

    // Store file path
    m_filePath = filePath;

    // stb_image loads images with origin at top-left, but OpenGL expects bottom-left
    // So we flip the image vertically during loading
    stbi_set_flip_vertically_on_load(true);

    // Load image data
    unsigned char *data = stbi_load(filePath.c_str(), &m_width, &m_height, &m_channels, 0);
    if (!data)
    {
        std::cerr << "Failed to load texture: " << filePath << std::endl;
        std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
        return false;
    }

    std::cout << "Loaded texture: " << filePath << std::endl;
    std::cout << "  Size: " << m_width << "x" << m_height << std::endl;
    std::cout << "  Channels: " << m_channels << std::endl;

    // Create texture from loaded data
    bool success = CreateFromData(data, m_width, m_height, m_channels);

    // Free image data (we've copied it to GPU memory)
    stbi_image_free(data);

    return success;
}

bool Texture::CreateFromData(unsigned char *data, int width, int height, int channels)
{
    if (!data || width <= 0 || height <= 0)
    {
        std::cerr << "Invalid texture data provided" << std::endl;
        return false;
    }

    // Store texture properties
    m_width = width;
    m_height = height;
    m_channels = channels;

    // Generate OpenGL texture object
    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_2D, m_textureID);

    // Determine OpenGL format based on number of channels
    GLenum format;
    switch (channels)
    {
    case 1:
        format = GL_RED; // Grayscale
        break;
    case 3:
        format = GL_RGB; // RGB
        break;
    case 4:
        format = GL_RGBA; // RGBA
        break;
    default:
        std::cerr << "Unsupported texture format: " << channels << " channels" << std::endl;
        Cleanup();
        return false;
    }

    // Upload texture data to GPU
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    // Set texture parameters for good quality
    SetTextureParameters();

    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);

    std::cout << "Created OpenGL texture with ID: " << m_textureID << std::endl;

    return true;
}

void Texture::Bind(unsigned int slot) const
{
    if (m_textureID == 0)
    {
        std::cerr << "Warning: Attempting to bind invalid texture" << std::endl;
        return;
    }

    // Activate texture unit
    glActiveTexture(GL_TEXTURE0 + slot);

    // Bind texture to the active unit
    glBindTexture(GL_TEXTURE_2D, m_textureID);
}

void Texture::Unbind(unsigned int slot)
{
    // Activate texture unit
    glActiveTexture(GL_TEXTURE0 + slot);

    // Unbind any texture
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::SetTextureParameters()
{
    // Apply current filter mode
    ApplyFilterParameters();
    // Wrapping (keep existing behavior)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void Texture::Cleanup()
{
    if (m_textureID != 0)
    {
        std::cout << "Destroying texture with ID: " << m_textureID << std::endl;
        glDeleteTextures(1, &m_textureID);
        m_textureID = 0;
    }

    m_width = 0;
    m_height = 0;
    m_channels = 0;
    m_filePath.clear();
}

void Texture::ApplyFilterParameters() const
{
    GLint filter = (s_currentFilterMode == FilterMode::Linear) ? GL_LINEAR : GL_NEAREST;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
}

void Texture::SetGlobalFilterMode(FilterMode mode)
{
    if (s_currentFilterMode == mode)
        return;
    s_currentFilterMode = mode;
    // Reapply to all live textures
    for (auto *tex : s_allTextures)
    {
        if (!tex || !tex->m_textureID)
            continue;
        glBindTexture(GL_TEXTURE_2D, tex->m_textureID);
        tex->ApplyFilterParameters();
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::FilterMode Texture::GetGlobalFilterMode()
{
    return s_currentFilterMode;
}
