#ifndef SPACEOBJECTS_MODEL_H
#define SPACEOBJECTS_MODEL_H

#include <string>
#include <vector>
#include <assimp/scene.h>

#include "BBox.h"
#include "Object.h"

class Model {
    std::string model_location;

    void process_object(const aiNode* node, const aiScene* scene);

    void process_textures(const aiScene* scene);

 public:
    std::vector<Object> objects;
    std::vector<Material> materials;

    BBox bbox;

    glm::vec3 world_pos;
    glm::mat4 rot;
    float scale_coef = 1.0;

    float damage = 10.0;

    Model() = default;

    explicit Model(const std::string& path);

    void move(const glm::vec3& translation) {
        world_pos += translation;
    }

    void rotate(float angle, const glm::vec3& axis) {
        rot = glm::rotate(rot, angle, axis);
    }

    void scale(float coef) {
        scale_coef *= coef;
    }

    glm::mat4 getWorldTransform() const {
        return glm::translate(glm::mat4(1.0f), world_pos) * rot * glm::scale(glm::mat4(1.0f), glm::vec3(scale_coef));
    }

    BBox getBBox() const {
        return BBox(bbox.min + world_pos, bbox.max + world_pos);
    }
};

class Asteroid : public Model {
public:
    glm::vec3 velocity;

    Asteroid(const Model& model, const glm::vec3& velocity) :
        Model(model),
        velocity(velocity) {}

    void moveAuto() {
        world_pos += velocity;
    }
};

#endif //SPACEOBJECTS_MODEL_H
