#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;

uniform mat4 transform;   // proj * model

out vec2 vUV;

void main() {
    vUV = aUV;
    gl_Position = transform * vec4(aPos, 0.0, 1.0);
}
