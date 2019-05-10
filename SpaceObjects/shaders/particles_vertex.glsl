#version 330

in vec3 vertex;

out vec3 point_position;

void main() {
    point_position = vertex;
}
