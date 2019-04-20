#ifndef SPACEOBJECTS_MODEL_H
#define SPACEOBJECTS_MODEL_H

#include <string>
#include <vector>
#include <assimp/scene.h>

#include "Object.h"

class Model {
 public:
    std::vector<Object> objects;

    glm::vec3 world_pos;
    glm::mat4 rot;

    explicit Model(const std::string& path);

    void process_object(const aiNode* node, const aiScene* scene);

    void move(const glm::vec3& translation) {
        world_pos += translation;
    }

    void rotate(float angle, const glm::vec3& axis) {
        rot = glm::rotate(rot, angle, axis);
    }

    glm::mat4 getWorldTransform() const {
        return glm::translate(glm::mat4(1.0f), world_pos) * rot;
    }
};

#endif //SPACEOBJECTS_MODEL_H
