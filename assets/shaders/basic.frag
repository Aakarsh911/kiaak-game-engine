#version 330 core

// Input from vertex shader
in vec2 TexCoord;

// Output color
out vec4 FragColor;

// Uniforms
uniform sampler2D u_texture;
uniform vec4 u_color;

void main()
{
    // Sample texture (will be white if no texture bound)
    vec4 texColor = texture(u_texture, TexCoord);
    
    // Multiply texture color by uniform color for tinting/solid colors
    FragColor = texColor * u_color;
}
