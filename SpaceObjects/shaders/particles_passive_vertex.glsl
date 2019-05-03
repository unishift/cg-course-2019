#version 330

in vec3 vertex;

uniform mat4 world_transform;
uniform mat4 perspective_transform;

const int radius = 20;

void main() {
    vec4 position4 = mod(world_transform * vec4(vertex, 1.0f) + radius, 2 * radius) - radius;
    gl_Position = perspective_transform * position4;
}
