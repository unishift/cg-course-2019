#include "ModelFactories.h"

#include <glm/gtx/norm.hpp>

ModelFactory::ModelFactory() {
    model_path = {
        {ModelName::E45_AIRCRAFT, "models/E-45-Aircraft/E 45 Aircraft_obj.obj"},
        {ModelName::REPVENATOR, "models/Venator/export.obj"},
        {ModelName::FIGHTER, "models/fighter/sci-fi_fighter.obj"},
        {ModelName::DEATHROW, "models/deathrow/DeathRow.obj"},
        {ModelName::MYST_ASTEROID, "models/mysterious_asteroid/A2.obj"},
        {ModelName::ASTEROID1, "models/asteroid1/asteroid1.obj"},
    };

    // Buffer all models
    for (const auto& pair : model_path) {
        const auto& model_name = pair.first;
        const auto& path = pair.second;

        model_buffer[pair.first] = Model(path);
    }
}

Model
ModelFactory::get_model(ModelName model_name, const glm::vec3 &position, const glm::vec3 &rotation, float scale) const {
    Model model = model_buffer.at(model_name);

    model.scale(scale);
    model.move(position);
    if (rotation != glm::vec3(0.0f)) {
        const float angle = glm::l2Norm(rotation);
        const auto axis = rotation / angle;

        model.rotate(angle, axis);
    }
    return model;
}
