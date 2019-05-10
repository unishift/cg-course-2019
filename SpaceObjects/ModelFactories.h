#ifndef SPACEOBJECTS_MODELFACTORIES_H
#define SPACEOBJECTS_MODELFACTORIES_H

#include "Model.h"

#include <map>

enum ModelName {
    E45_AIRCRAFT,
    ROCKET,
    REPVENATOR,
    FIGHTER,
    DEATHROW,
    MYST_ASTEROID,
    ASTEROID1,
};

class ModelFactory {
    std::map<ModelName, std::string> model_path;
    std::map<ModelName, Model> model_buffer;
public:
    ModelFactory();

    Model get_model(ModelName model_name, const glm::vec3& position = glm::vec3(0.0f), const glm::vec3& rotation = glm::vec3(0.0f), float scale = 1.0f) const;

    Model get_random_enemy(const glm::vec3& position) const {
        const float x = rand() % 50 - 25;
        const float y = rand() % 50 - 25;
        const auto src = glm::vec3(x, y, -200.0f) + position;

        const auto choice = static_cast<ModelName>(ModelName::REPVENATOR + rand() % 3);

        return get_model(choice, src);
    }

    Asteroid get_random_asteroid(const glm::vec3& position, const glm::vec3& target) const {
        const float x = rand() % 50 - 25;
        const float y = rand() % 50 - 25;
        const float scale = (rand() % 50) / 50.0f + 0.5f;
        const auto src = glm::vec3(x, y, -200.0f) + position;
        const glm::vec3 velocity = glm::normalize(target - src);

        const auto choice = static_cast<ModelName>(ModelName::MYST_ASTEROID + rand() % 2);

        return Asteroid(get_model(choice, src, glm::vec3(0.0f), scale), velocity);
    }
};

#endif //SPACEOBJECTS_MODELFACTORIES_H
