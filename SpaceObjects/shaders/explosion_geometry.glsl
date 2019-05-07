#version 330

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec4 point_positions[];
in vec2 texture_coords[];

out vec2 tex_coords;

uniform float magnitude;

void main() {
    vec3 a = vec3(point_positions[0] - point_positions[1]);
    vec3 b = vec3(point_positions[1] - point_positions[2]);
    vec3 norm = normalize(cross(a, b));

    gl_Position = point_positions[0] + vec4(norm * magnitude, 0.0f);
    tex_coords = texture_coords[0];
    EmitVertex();

    gl_Position = point_positions[1] + vec4(norm * magnitude, 0.0f);
    tex_coords = texture_coords[1];
    EmitVertex();

    gl_Position = point_positions[2] + vec4(norm * magnitude, 0.0f);
    tex_coords = texture_coords[2];
    EmitVertex();

    EndPrimitive();
}
