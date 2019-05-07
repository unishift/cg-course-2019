#version 330

layout (points) in;
layout (line_strip, max_vertices = 2) out;

in vec3 point_position[];

out vec4 color;

uniform mat4 world_transform;
uniform mat4 perspective_transform;
uniform vec3 velocity;

const int radius = 20;

void main() {
    vec3 position = point_position[0];

    vec4 position4 = mod(world_transform * vec4(position, 1.0f) + radius, 2 * radius) - radius;
    gl_Position = perspective_transform * position4;
    color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    EmitVertex();

    gl_Position = perspective_transform * (position4 - world_transform * vec4(0.75f * velocity, 0.0f));
    color = 0.1f * vec4(1.0f, 1.0f, 1.0f, 1.0f);
    EmitVertex();

    EndPrimitive();
}
