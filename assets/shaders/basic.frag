#version 330 core

// Input from vertex shader
in vec2 TexCoord;

// Output color
out vec4 FragColor;

// Texture sampler
uniform sampler2D texture1;

void main()
{
    // Output the texture color
    FragColor = texture(texture1, TexCoord);
}
