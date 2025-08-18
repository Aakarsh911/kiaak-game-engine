#version 330 core

// Input vertex data
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

// Output data that will be passed to fragment shader
out vec2 TexCoord;

// Uniforms for transformations
uniform mat4 u_model;

void main()
{
    // Calculate final position - just use model matrix (normalized device coordinates)
    gl_Position = u_model * vec4(aPos, 0.0, 1.0);
    
    // Pass texture coordinates to fragment shader
    TexCoord = aTexCoord;
}
