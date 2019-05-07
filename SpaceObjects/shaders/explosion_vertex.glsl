#version 330

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 texture_coordinates;

uniform mat4 transform;

out vec4 point_positions;
out vec2 texture_coords;

void main() {
    point_positions = transform * vec4(vertex, 1.0f);
    texture_coords = texture_coordinates;
}
