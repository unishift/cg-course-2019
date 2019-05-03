#version 330 core

in vec2 texture_coords;

out vec4 color;

uniform vec4 diffuse_color;
uniform sampler2D Texture;
uniform int use_texture;

void main() {
    if (use_texture != 0) {
        color = texture(Texture, texture_coords);
    } else {
        color = diffuse_color;
    }
}