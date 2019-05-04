#ifndef SPACEOBJECTS_MODELFACTORIES_H
#define SPACEOBJECTS_MODELFACTORIES_H

#include "Model.h"

#include <map>
#include <glm/gtx/norm.hpp>

enum class ModelName {
    E45_AIRCRAFT,
    ENTERPRISE_NCC1701D,
    REPVENATOR,
};

std::map<ModelName, Model> model_buffer;

static inline
Model create_model(ModelName model_name, const glm::vec3& position = glm::vec3(0.0f), const glm::vec3& rotation = glm::vec3(0.0f), float scale = 1.0f) {
    Model model;

    if (model_buffer.count(model_name) == 0) {
        switch (model_name) {
            case ModelName::E45_AIRCRAFT:
                model = Model("models/E-45-Aircraft/E 45 Aircraft_obj.obj");
                model.rotate(M_PI, {0, 1, 0});
                model.move({0, 0, 3});
                break;
            case ModelName::ENTERPRISE_NCC1701D:
                model = Model("models/Enterprise NCC 1701 D/enterprise1701d.obj");
                model.move({-44.15, -4, 8});
                break;
            case ModelName::REPVENATOR:
                model = Model("models/Venator/export.obj");
                break;
        }
        model_buffer[model_name] = model;
    } else {
        model = model_buffer[model_name];
    }

    model.scale(scale);
    model.move(position);
    if (rotation != glm::vec3(0.0f)) {
        const float angle = glm::l2Norm(rotation);
        const auto axis = rotation / angle;

        model.rotate(angle, axis);
    }
    return model;
}

#endif //SPACEOBJECTS_MODELFACTORIES_H
