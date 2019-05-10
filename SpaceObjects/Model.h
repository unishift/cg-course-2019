#ifndef SPACEOBJECTS_MODEL_H
#define SPACEOBJECTS_MODEL_H

#include <string>
#include <vector>
#include <assimp/scene.h>
#include <glm/gtx/quaternion.hpp>

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
        const auto transform = getWorldTransform();
        return BBox(transform * glm::vec4(bbox.min, 1.0f), transform * glm::vec4(bbox.max, 1.0f));
    }

    bool dead = false;
    int death_countdown = 60;

    bool die() {
        // ...

        death_countdown--;
        return death_countdown == 0;
    }
};

class Asteroid : public Model {
public:
    glm::vec3 velocity;

    Asteroid(const Model& model, const glm::vec3& velocity) :
        Model(model),
        velocity(velocity) {

        const auto direction = glm::normalize(velocity);
        const auto q = glm::rotation({0.0f, 0.0f, 1.0f}, direction);

        rot = glm::toMat4(q) * rot;
    }

    void moveAuto() {
        world_pos += velocity;
    }
};

#endif //SPACEOBJECTS_MODEL_H
