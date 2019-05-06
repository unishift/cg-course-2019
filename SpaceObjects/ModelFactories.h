#ifndef SPACEOBJECTS_MODELFACTORIES_H
#define SPACEOBJECTS_MODELFACTORIES_H

#include "Model.h"

#include <map>

enum class ModelName {
    E45_AIRCRAFT,
    ENTERPRISE_NCC1701D,
    REPVENATOR,
    MYST_ASTEROID,
    ASTEROID1,
};

class ModelFactory {
    std::map<ModelName, std::string> model_path;
    std::map<ModelName, Model> model_buffer;
public:
    ModelFactory();

    Model get_model(ModelName model_name, const glm::vec3& position = glm::vec3(0.0f), const glm::vec3& rotation = glm::vec3(0.0f), float scale = 1.0f) const;
};

#endif //SPACEOBJECTS_MODELFACTORIES_H
