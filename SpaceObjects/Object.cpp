#include "Object.h"

void Object::init() {
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

Object::Object(const std::vector<float> &vertices, const std::vector<GLuint> &elements) :
    vertices(vertices),
    elements(elements),
    world_pos(0.0f, 0.0f, 0.0f),
    rot(1.0f) {

    init();
}

Object::Object(const aiMesh* mesh) :
    world_pos(0.0f, 0.0f, 0.0f),
    rot(1.0f) {
    // Vertices
    for (int i = 0; i < mesh->mNumVertices; i++) {
        const auto& vertex = mesh->mVertices[i];

        vertices.push_back(vertex.x);
        vertices.push_back(vertex.y);
        vertices.push_back(vertex.z);
    }

    // Indices
    for (int i = 0; i < mesh->mNumFaces; i++) {
        const auto& face = mesh->mFaces[i];

        for (int j = 0; j < face.mNumIndices; j++) {
            elements.push_back(face.mIndices[j]);
        }
    }

    init();
}
