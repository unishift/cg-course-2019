#ifndef SPACEOBJECTS_CAMERA_H
#define SPACEOBJECTS_CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

enum class CameraMode {
    FIRST_PERSON,
    THIRD_PERSON,
};

struct Camera {
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 up;
    glm::vec3 right;

    glm::quat rot;

    CameraMode mode;

    Camera() :
        position(0.0f, 0.0f, 0.0f),
        direction(0.0f, 0.0f, -10.0f),
        up(0.0f, 1.0f, 0.0f),
        right(1.0f, 0.0f, 0.0f),
        mode(CameraMode::THIRD_PERSON) {}

    void move(const glm::vec3& v) {
        position += v;
    }

    glm::mat4 getViewTransform() const {
        if (mode == CameraMode::FIRST_PERSON) {
            const auto tmp_position = position + glm::vec3(0.0f, 0.5f, -1.0f);
            return glm::lookAt(tmp_position, rot * (tmp_position + direction), rot * up);
        } else if (mode == CameraMode::THIRD_PERSON) {
            const auto tmp_position = position + glm::vec3(0.0f, 3.0f, 0.0f);
            return glm::lookAt(tmp_position - rot * direction, tmp_position, rot * up);
        }
    }
};

#endif //SPACEOBJECTS_CAMERA_H
