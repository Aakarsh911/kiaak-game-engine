#pragma once

#include <glad/glad.h>
#include <string>
#include <unordered_set>

/**
 * Texture class manages OpenGL textures for 2D rendering
 *
 * This class handles:
 * - Loading images from files using stb_image
 * - Creating OpenGL texture objects
 * - Managing texture parameters (filtering, wrapping)
 * - Binding textures for rendering
 *
 * Supported formats: PNG, JPG, BMP, TGA
 */
class Texture
{
private:
    GLuint m_textureID;     // OpenGL texture object ID
    int m_width;            // Texture width in pixels
    int m_height;           // Texture height in pixels
    int m_channels;         // Number of color channels (3=RGB, 4=RGBA)
    std::string m_filePath; // Path to source image file

    // Apply current global filter mode to this texture (if valid)
    void ApplyFilterParameters() const;

public:
    // Texture filtering mode
    enum class FilterMode
    {
        Linear = 0,
        Nearest = 1
    };

    /**
     * Default constructor - creates an empty texture
     */
    Texture();

    /**
     * Constructor that loads texture from file
     * @param filePath Path to image file to load
     */
    Texture(const std::string &filePath);

    /**
     * Destructor - cleans up OpenGL texture
     */
    ~Texture();

    /**
     * Load texture from image file
     * @param filePath Path to image file
     * @return true if loading succeeded, false otherwise
     */
    bool LoadFromFile(const std::string &filePath);

    /**
     * Create texture from raw pixel data
     * @param data Pointer to pixel data
     * @param width Texture width
     * @param height Texture height
     * @param channels Number of color channels (3 or 4)
     * @return true if creation succeeded, false otherwise
     */
    bool CreateFromData(unsigned char *data, int width, int height, int channels);

    /**
     * Bind this texture for rendering
     * @param slot Texture unit to bind to (0-31, default 0)
     */
    void Bind(unsigned int slot = 0) const;

    /**
     * Unbind any texture from the specified slot
     * @param slot Texture unit to unbind (default 0)
     */
    static void Unbind(unsigned int slot = 0);

    /**
     * Get texture width
     * @return Width in pixels
     */
    int GetWidth() const { return m_width; }

    /**
     * Get texture height
     * @return Height in pixels
     */
    int GetHeight() const { return m_height; }

    /**
     * Get number of color channels
     * @return 3 for RGB, 4 for RGBA
     */
    int GetChannels() const { return m_channels; }

    /**
     * Get OpenGL texture ID
     * @return OpenGL texture object ID
     */
    GLuint GetID() const { return m_textureID; }

    /**
     * Check if texture is valid (loaded successfully)
     * @return true if texture is valid, false otherwise
     */
    bool IsValid() const { return m_textureID != 0; }

    /**
     * Get file path of loaded texture
     * @return Path to source image file
     */
    const std::string &GetFilePath() const { return m_filePath; }

    // -------- Global filtering control --------
    // Set the global filter mode (applies to all existing textures immediately)
    static void SetGlobalFilterMode(FilterMode mode);
    static FilterMode GetGlobalFilterMode();

private:
    /**
     * Set up OpenGL texture parameters (filtering, wrapping)
     */
    void SetTextureParameters();

    /**
     * Clean up OpenGL resources
     */
    void Cleanup();

    // Registry of all live Texture instances to support global filter changes
    static std::unordered_set<Texture *> s_allTextures;
    static FilterMode s_currentFilterMode;
};
