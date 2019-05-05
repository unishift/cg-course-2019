#version 330

layout (location = 0) in vec2 vertex;

uniform vec2 position;

void main() {
    gl_Position = vec4(vertex + position, 0.0f, 1.0f);
}
