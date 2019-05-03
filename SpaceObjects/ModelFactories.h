#ifndef SPACEOBJECTS_MODELFACTORIES_H
#define SPACEOBJECTS_MODELFACTORIES_H

#include "Model.h"

enum class ModelName {
    E45_AIRCRAFT,
    ENTERPRISE_NCC1701D,
};

static inline
Model create_model(ModelName model_name, const glm::vec3 position = {0, 0, 0}, float scale = 0) {
    Model model;

    switch (model_name) {
        case ModelName::E45_AIRCRAFT:
            model =  Model("models/E-45-Aircraft/E 45 Aircraft_obj.obj");
            model.rotate(M_PI, {0, 1, 0});
            break;
        case ModelName::ENTERPRISE_NCC1701D:
            model =  Model("models/Enterprise NCC 1701 D/enterprise1701d.obj");
            model.move({-44.15, -4, 8});
            break;

    }

    model.move(position);
    return model;
}

#endif //SPACEOBJECTS_MODELFACTORIES_H
