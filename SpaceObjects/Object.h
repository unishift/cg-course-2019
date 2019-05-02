#ifndef SPACEOBJECTS_OBJECT_H
#define SPACEOBJECTS_OBJECT_H

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/mesh.h>
#include "common.h"

class Object {
    GLuint VAO, VBO, EBO, TBO;

    void init();
public:
    std::vector<GLfloat> vertices;
    std::vector<GLuint> elements;
    std::vector<GLfloat> texture_coords;
    unsigned int material_index;

    glm::vec3 world_pos;
    glm::mat4 rot;

    Object(const std::vector<float>& vertices, const std::vector<GLuint>& elements);

    explicit Object(const aiMesh* mesh, GLuint texture_index);

    void move(const glm::vec3& translation) {
        world_pos += translation;
    }

    void rotate(float angle, const glm::vec3& axis) {
        rot = glm::rotate(rot, angle, axis);
    }

    glm::mat4 getWorldTransform() const {
        return glm::translate(glm::mat4(1.0f), world_pos) * rot;
    }

    void draw() const {
        glBindVertexArray(VAO);
        if (material_index != -1) {
            glBindTexture(GL_TEXTURE_2D, material_index);
        }
        GL_CHECK_ERRORS;
        glDrawElements(GL_TRIANGLES, elements.size(), GL_UNSIGNED_INT, nullptr);
        GL_CHECK_ERRORS;
        glBindVertexArray(0);
    }
};

#endif //SPACEOBJECTS_OBJECT_H
