#version 330

in vec2 tex_coords;
out vec4 color;

uniform sampler2D Texture;
uniform vec3 text_color;

void main() {
    color = vec4(text_color, texture(Texture, tex_coords).r);
}
