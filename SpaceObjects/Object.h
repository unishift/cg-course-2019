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

    void init();
public:
    std::vector<GLfloat> vertices;
    std::vector<GLuint> elements;
    std::vector<GLfloat> texture_coords;

    glm::vec3 world_pos;
    glm::mat4 rot;

    Object(const std::vector<float>& vertices, const std::vector<GLuint>& elements, const Material& material);

    explicit Object(const aiMesh* mesh, const Material& material);

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

    bool haveTexture() const {
        return material.diffuse_texture != 0;
    }

    virtual void draw() const {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, material.diffuse_texture);
        glBindVertexArray(VAO);

        GL_CHECK_ERRORS;
        glDrawElements(GL_TRIANGLES, elements.size(), GL_UNSIGNED_INT, nullptr);
        GL_CHECK_ERRORS;
        glBindVertexArray(0);
    }
};

class SkyBox : private Object {
public:

    explicit SkyBox(GLuint texture_index) :
        // Create cube object
        Object({
                // front
                -1.0, -1.0,  1.0,
                1.0, -1.0,  1.0,
                1.0,  1.0,  1.0,
                -1.0,  1.0,  1.0,
                // back
                -1.0, -1.0, -1.0,
                1.0, -1.0, -1.0,
                1.0,  1.0, -1.0,
                -1.0,  1.0, -1.0,
            }, {
                // front
                0, 1, 2,
                2, 3, 0,
                // right
                1, 5, 6,
                6, 2, 1,
                // back
                7, 6, 5,
                5, 4, 7,
                // left
                4, 0, 3,
                3, 7, 4,
                // bottom
                4, 5, 1,
                1, 0, 4,
                // top
                3, 2, 6,
                6, 7, 3,
            }, Material(texture_index)
        ) { }

    void draw() const override {
//        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, material.diffuse_texture);
        GL_CHECK_ERRORS;
        glBindVertexArray(VAO);
        GL_CHECK_ERRORS;

        glDrawElements(GL_TRIANGLES, elements.size(), GL_UNSIGNED_INT, nullptr);
        GL_CHECK_ERRORS;
        glBindVertexArray(0);
    }

    static SkyBox create(const std::string& path);
};

#endif //SPACEOBJECTS_OBJECT_H
