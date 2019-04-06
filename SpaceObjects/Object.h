#ifndef SPACEOBJECTS_OBJECT_H
#define SPACEOBJECTS_OBJECT_H

#include <vector>
#include <glad/glad.h>

class Object {
    GLuint VAO, VBO, EBO;
public:
    std::vector<GLfloat> vertices;
    std::vector<GLuint> elements;

    Object(const std::vector<float>& vertices, const std::vector<GLuint>& elements) :
        vertices(vertices),
        elements(elements) {

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

    void draw() const {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, elements.size(), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }
};

#endif //SPACEOBJECTS_OBJECT_H
