#include "Object.h"

#include <random>

void Object::init() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &TBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(GLuint), elements.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, TBO);
    glBufferData(GL_ARRAY_BUFFER, texture_coords.size() * sizeof(GLfloat), texture_coords.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindVertexArray(0);
}

Object::Object(const std::vector<float> &vertices, const std::vector<GLuint> &elements, const Material& material) :
    material(material),
    vertices(vertices),
    elements(elements),
    world_pos(0.0f, 0.0f, 0.0f),
    rot(1.0f) {

    init();
}

Object::Object(const aiMesh* mesh, const Material& material) :
    material(material),
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

SkyBox SkyBox::create(const std::array<std::string, 6>& file_names) {
    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);

    for (int i = 0; i < file_names.size(); i++) {
        uint image = ilGenImage();
        ilBindImage(image);
        const auto status = ilLoadImage(file_names[i].c_str());
        if (!status) {
            std::cerr << "Error loading skybox: " << ilGetError() << std::endl;
        }

        const int w = ilGetInteger(IL_IMAGE_WIDTH);
        const int h = ilGetInteger(IL_IMAGE_HEIGHT);
        const int format = ilGetInteger(IL_IMAGE_FORMAT);
        const int type = ilGetInteger(IL_IMAGE_TYPE);

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0, GL_RGB, w, h, 0, format, type, ilGetData());

        ilDeleteImage(image);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return SkyBox(texture_id);
}

Particles::Particles(int nb_particles) {
    // Random stuff
    std::random_device rd;
    std::mt19937 gen(rd());
    auto randomizer = std::uniform_real_distribution<float>(-50.0f, 50.0f);

    vertices.reserve(nb_particles * 3);
    for (int i = 0; i < 3 * nb_particles; i++) {
        vertices.push_back(randomizer(gen));
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 3 * nb_particles, vertices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindVertexArray(0);
}
