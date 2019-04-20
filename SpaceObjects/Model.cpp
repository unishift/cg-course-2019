#include "Model.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <iostream>

Model::Model(const std::string& path) :
    world_pos(0.0f, 0.0f, 0.0f),
    rot(1.0f) {

    Assimp::Importer importer;

    auto scene = importer.ReadFile(path.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs);
    if (scene == nullptr || scene->mRootNode == nullptr) {
        std::cerr << "Couldn't read model" << std::endl;
    } else {
        process_object(scene->mRootNode, scene);
    }
}
void Model::process_object(const aiNode* node, const aiScene* scene) {
    for (int i = 0; i < node->mNumMeshes; i++) {
        const auto mesh = scene->mMeshes[node->mMeshes[i]];

        objects.emplace_back(mesh);
    }

    for (int i = 0; i < node->mNumChildren; i++) {
        process_object(node->mChildren[i], scene);
    }
}
