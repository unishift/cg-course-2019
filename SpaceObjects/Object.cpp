#include "Object.h"

void Object::init() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &TBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLuint), nullptr);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(GLuint), elements.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, TBO);
    glBufferData(GL_ARRAY_BUFFER, texture_coords.size() * sizeof(GLfloat), texture_coords.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);

    glBindVertexArray(0);
}

Object::Object(const std::vector<float> &vertices, const std::vector<GLuint> &elements) :
    vertices(vertices),
    elements(elements),
    world_pos(0.0f, 0.0f, 0.0f),
    rot(1.0f) {

    init();
}

Object::Object(const aiMesh* mesh, GLuint texture_index) :
    material_index(texture_index),
    world_pos(0.0f, 0.0f, 0.0f),
    rot(1.0f) {
    // Vertices
    for (int i = 0; i < mesh->mNumVertices; i++) {
        const auto& vertex = mesh->mVertices[i];

        vertices.push_back(vertex.x);
        vertices.push_back(vertex.y);
        vertices.push_back(vertex.z);

        // Texture coordinates
        texture_coords.push_back(mesh->mTextureCoords[0][i].x);
        texture_coords.push_back(mesh->mTextureCoords[0][i].y);
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
