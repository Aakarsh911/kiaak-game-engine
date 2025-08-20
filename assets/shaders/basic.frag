#version 330 core
in vec2 vUV;
out vec4 FragColor;

uniform sampler2D ourTexture;

void main() {
    vec4 texColor = texture(ourTexture, vUV);
    
    // Discard pixels with very low alpha to prevent gray squares
    if (texColor.a < 0.1) {
        discard;
    }
    
    FragColor = texColor;
}
