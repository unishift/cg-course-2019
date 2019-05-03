#version 330

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 texture_coordinates;

uniform mat4 transform;

out vec2 texture_coords;

void main() {
    gl_Position  = transform * vec4(vertex, 1.0f);
    texture_coords = texture_coordinates;
}
