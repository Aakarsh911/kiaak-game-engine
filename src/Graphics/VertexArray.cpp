#include "Graphics/VertexArray.hpp"
#include <iostream>

namespace Kiaak {

VertexArray::VertexArray() {
    // Generate a Vertex Array Object ID
    glGenVertexArrays(1, &m_arrayID);
    std::cout << "Created VertexArray with ID: " << m_arrayID << std::endl;
}

VertexArray::~VertexArray() {
    std::cout << "Destroying VertexArray with ID: " << m_arrayID << std::endl;
    glDeleteVertexArrays(1, &m_arrayID);
}

void VertexArray::Bind() const {
    // Bind this VAO as the active one
    glBindVertexArray(m_arrayID);
}

void VertexArray::Unbind() const {
    // Unbind by setting 0 as active VAO
    glBindVertexArray(0);
}

void VertexArray::AddAttribute(unsigned int index, unsigned int count, unsigned int type, 
                              bool normalized, unsigned int stride, const void* offset) {
    // Must be bound before adding attributes
    Bind();
    
    // Configure the vertex attribute
    glVertexAttribPointer(index, count, type, normalized ? GL_TRUE : GL_FALSE, stride, offset);
    
    // Store attribute info for debugging
    VertexAttribute attr = {index, count, type, normalized, stride, offset};
    m_attributes.push_back(attr);
    
    std::cout << "Added attribute " << index << ": " << count << " components of type " 
              << (type == GL_FLOAT ? "GL_FLOAT" : "OTHER") 
              << ", stride: " << stride << std::endl;
}

void VertexArray::EnableAttribute(unsigned int index) {
    // Must be bound before enabling attributes
    Bind();
    glEnableVertexAttribArray(index);
    std::cout << "Enabled vertex attribute " << index << std::endl;
}

} // namespace Kiaak
