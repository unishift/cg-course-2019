#ifndef SPACEOBJECTS_OBJECT_H
#define SPACEOBJECTS_OBJECT_H

#include <array>
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/mesh.h>
#include <il.h>

#include "common.h"
#include "Material.h"

class Object {
protected:
    GLuint VAO, VBO, EBO, TBO;
    Material material;

public:
    std::vector<GLfloat> vertices;
    std::vector<GLuint> elements;
    std::vector<GLfloat> texture_coords;

    glm::vec3 world_pos;
    glm::mat4 rot;

    Object(const std::vector<float>& vertices, const std::vector<GLuint>& elements, const std::vector<GLfloat>& texture_coords, const Material& material);

    void move(const glm::vec3& translation) {
        world_pos += translation;
    }

    void rotate(float angle, const glm::vec3& axis) {
        rot = glm::rotate(rot, angle, axis);
    }

    glm::mat4 getWorldTransform() const {
        return glm::translate(glm::mat4(1.0f), world_pos) * rot;
    }

    glm::vec4 getDiffuseColor() const {
        return material.diffuse_color;
    }

    float getOpacity() const {
        return material.opacity;
    }

    bool haveTexture() const {
        return material.diffuse_texture != 0;
    }

    void draw() const {
        glBindTexture(GL_TEXTURE_2D, material.diffuse_texture);
        glBindVertexArray(VAO);

        GL_CHECK_ERRORS;
        glDrawElements(GL_TRIANGLES, elements.size(), GL_UNSIGNED_INT, nullptr);
        GL_CHECK_ERRORS;
        glBindVertexArray(0);
    }

    static Object create(const aiMesh* mesh, const Material& material);
};

class SkyBox {
    GLuint VAO, VBO, EBO;
    GLuint texture_index;

public:
    std::vector<GLfloat> vertices;
    std::vector<GLuint> elements;

    explicit SkyBox(GLuint texture_index);

    void draw() const {
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture_index);
        GL_CHECK_ERRORS;
        glBindVertexArray(VAO);
        GL_CHECK_ERRORS;

        glDrawElements(GL_TRIANGLES, elements.size(), GL_UNSIGNED_INT, nullptr);
        GL_CHECK_ERRORS;
        glBindVertexArray(0);
    }

    static SkyBox create(const std::array<std::string, 6>& file_names);
};

class Particles {
    GLuint VAO, VBO;
    std::vector<GLfloat> vertices;
public:

    explicit Particles(int nb_particles);

    void draw() const {
        glBindVertexArray(VAO);

        glDrawArrays(GL_POINTS, 0, vertices.size() / 3);
        GL_CHECK_ERRORS;

        glBindVertexArray(0);
    }
};

class Crosshair {
    GLuint VAO, VBO;
    GLfloat vertices[6] {
        0.0f, 0.02f * sqrtf(3.0f),
        -0.02f, -0.02f,
        0.02f, -0.02f,
    };

public:

    Crosshair();

    void draw() const {
        glBindVertexArray(VAO);

        glDrawArrays(GL_LINE_LOOP, 0, 3);
        GL_CHECK_ERRORS;

        glBindVertexArray(0);
    }
};

#endif //SPACEOBJECTS_OBJECT_H
