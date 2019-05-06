#include "ModelFactories.h"

#include <glm/gtx/norm.hpp>

ModelFactory::ModelFactory() {
    model_path = {
        {ModelName::E45_AIRCRAFT, "models/E-45-Aircraft/E 45 Aircraft_obj.obj"},
        {ModelName::ENTERPRISE_NCC1701D, "models/Enterprise NCC 1701 D/enterprise1701d.obj"},
        {ModelName::REPVENATOR, "models/Venator/export.obj"},
        {ModelName::MYST_ASTEROID, "models/mysterious_asteroid/A2.obj"},
        {ModelName::ASTEROID1, "models/asteroid1/planet3.obj"},
    };

    // Buffer all models
    for (const auto& pair : model_path) {
        const auto& model_name = pair.first;
        const auto& path = pair.second;

        Model model(path);

        // Apply model-specific transformation
        // Some models have weird default position
        switch (model_name) {
            case ModelName::E45_AIRCRAFT:
                model.rotate(M_PI, {0, 1, 0});
                model.move({0, 0, 3});
                break;
            case ModelName::ENTERPRISE_NCC1701D:
                model.move({-44.15, -4, 8});
                break;
            case ModelName::REPVENATOR:
                break;
            case ModelName::MYST_ASTEROID:
                break;
        }

        model_buffer[pair.first] = model;
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
