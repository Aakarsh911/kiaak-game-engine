#include "Graphics/VertexBuffer.hpp"
#include <iostream>

namespace Kiaak
{

    VertexBuffer::VertexBuffer(const void *data, unsigned int size)
    {
        // Generate a buffer ID from OpenGL
        glGenBuffers(1, &m_bufferID);
        std::cout << "Created VertexBuffer with ID: " << m_bufferID << std::endl;

        // Bind this buffer as the active array buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_bufferID);

        // Upload data to the GPU
        // GL_STATIC_DRAW means we set the data once and draw many times
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

        std::cout << "Uploaded " << size << " bytes to GPU" << std::endl;
    }

    VertexBuffer::~VertexBuffer()
    {
        std::cout << "Destroying VertexBuffer with ID: " << m_bufferID << std::endl;
        glDeleteBuffers(1, &m_bufferID);
    }

    void VertexBuffer::Bind() const
    {
        // Tell OpenGL to use this buffer for array operations
        glBindBuffer(GL_ARRAY_BUFFER, m_bufferID);
    }

    void VertexBuffer::Unbind() const
    {
        // Unbind by setting 0 as the active buffer
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void VertexBuffer::SetData(const void *data, unsigned int size)
    {
        // Bind first, then update data
        Bind();
        glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
        // buffer data updated
    }

} // namespace Kiaak
