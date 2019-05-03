#ifndef SPACEOBJECTS_MATERIAL_H
#define SPACEOBJECTS_MATERIAL_H

#include <vector>
#include <glm/glm.hpp>

class Material {
public:
    GLuint diffuse_texture;
    glm::vec4 diffuse_color;

    explicit Material(GLuint diffuse_texture, const glm::vec4& diffuse_color = glm::vec4(1.0f)) :
        diffuse_texture(diffuse_texture),
        diffuse_color(diffuse_color) {}
};

#endif //SPACEOBJECTS_MATERIAL_H
