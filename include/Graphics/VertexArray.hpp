#pragma once

#include <glad/glad.h>
#include <vector>

namespace Kiaak {

// Structure to describe vertex attributes
struct VertexAttribute {
    unsigned int index;      // Attribute location (0, 1, 2, etc.)
    unsigned int count;      // Number of components (2 for vec2, 3 for vec3, etc.)
    unsigned int type;       // Data type (GL_FLOAT, GL_INT, etc.)
    bool normalized;         // Should values be normalized?
    unsigned int stride;     // Bytes between vertices
    const void* offset;      // Offset within vertex
};

class VertexArray {
public:
    VertexArray();
    ~VertexArray();

    void Bind() const;
    void Unbind() const;
    
    // Add a vertex attribute (position, color, texture coords, etc.)
    void AddAttribute(unsigned int index, unsigned int count, unsigned int type, 
                     bool normalized, unsigned int stride, const void* offset);
    
    // Enable a vertex attribute
    void EnableAttribute(unsigned int index);

private:
    unsigned int m_arrayID;
    std::vector<VertexAttribute> m_attributes;
};

} // namespace Kiaak
