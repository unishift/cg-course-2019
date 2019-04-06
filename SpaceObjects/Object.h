#ifndef SPACEOBJECTS_OBJECT_H
#define SPACEOBJECTS_OBJECT_H

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Object {
    GLuint VAO, VBO, EBO;
public:
    std::vector<GLfloat> vertices;
    std::vector<GLuint> elements;

    glm::vec3 world_pos;
    glm::mat4 rot;

    Object(const std::vector<float>& vertices, const std::vector<GLuint>& elements) :
        vertices(vertices),
        elements(elements),
        world_pos(0.0f, 0.0f, 0.0f),
        rot(1.0f) {

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(GLuint), elements.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLuint), nullptr);

        glBindVertexArray(0);
    }

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
        glDrawElements(GL_TRIANGLES, elements.size(), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }
};

#endif //SPACEOBJECTS_OBJECT_H
