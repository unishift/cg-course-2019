#include "Model.h"
#include "common.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <iostream>
#include <il.h>

Model::Model(const std::string& path) :
    world_pos(0.0f, 0.0f, 0.0f),
    rot(1.0f) {

    model_location = path.substr(0, path.find_last_of('/'));

    Assimp::Importer importer;

    const auto scene = importer.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs);
    if (scene == nullptr || scene->mRootNode == nullptr) {
        std::cerr << "Couldn't read model" << std::endl;
    } else {
        process_textures(scene);

        process_object(scene->mRootNode, scene);
    }
}

void Model::process_object(const aiNode* node, const aiScene* scene) {
    for (int i = 0; i < node->mNumMeshes; i++) {
        const auto mesh = scene->mMeshes[node->mMeshes[i]];

        objects.emplace_back(mesh, materials[mesh->mMaterialIndex]);
    }

    for (int i = 0; i < node->mNumChildren; i++) {
        process_object(node->mChildren[i], scene);
    }
}

GLuint read_texture(const std::string& path) {
    ILboolean devil_status;
    const ILuint image_id = ilGenImage();
    ilBindImage(image_id);

    devil_status = ilLoadImage(path.c_str());
    if (!devil_status) {
        std::cerr << "Failed to load image: " << path << std::endl;
    }

    devil_status = ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE);
    if (!devil_status) {
        std::cerr << "Failed to convert image: " << ilGetError() << std::endl;
    }

    const auto image_location = ilGetData();
    const auto format = ilGetInteger(IL_IMAGE_FORMAT);
    const auto width = ilGetInteger(IL_IMAGE_WIDTH);
    const auto height = ilGetInteger(IL_IMAGE_HEIGHT);

    GLuint texture_id;
    glGenTextures(1, &texture_id);
    GL_CHECK_ERRORS;

    glBindTexture(GL_TEXTURE_2D, texture_id);
    GL_CHECK_ERRORS;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    GL_CHECK_ERRORS;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GL_CHECK_ERRORS;

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image_location);
    GL_CHECK_ERRORS;

    glBindTexture(GL_TEXTURE_2D, 0);

    ilDeleteImage(image_id);

    return texture_id;
}

void Model::process_textures(const aiScene* scene) {
    for (const auto texture_type : {aiTextureType_DIFFUSE}) {
        for (int material_index = 0; material_index < scene->mNumMaterials; material_index++) {
            const auto material = scene->mMaterials[material_index];
            const auto num_textures = material->GetTextureCount(texture_type);

            aiColor3D diffuse_color;
            float opacity;
            material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse_color);
            material->Get(AI_MATKEY_OPACITY, opacity);
            const auto glm_diffuse_color = glm::vec4(diffuse_color.r, diffuse_color.g, diffuse_color.b, 1.0f);
            if (num_textures == 0) {
                materials.emplace_back(0, glm_diffuse_color, opacity);
            } else {
                aiString path;
                material->GetTexture(texture_type, 0, &path);

                const auto full_path = model_location + '/' + path.C_Str();
                materials.emplace_back(read_texture(full_path), glm_diffuse_color, opacity);
            }
        }
    }
}
