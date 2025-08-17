#pragma once

#include <glad/glad.h>

namespace Kiaak {

class VertexBuffer {
public:
    VertexBuffer(const void* data, unsigned int size);
    ~VertexBuffer();

    void Bind() const;
    void Unbind() const;
    
    // Update buffer data
    void SetData(const void* data, unsigned int size);

private:
    unsigned int m_bufferID;
};

} // namespace Kiaak
