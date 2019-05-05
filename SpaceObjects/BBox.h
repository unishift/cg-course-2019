#ifndef SPACEOBJECTS_BBOX_H
#define SPACEOBJECTS_BBOX_H

#include <glm/glm.hpp>
#include <glm/gtx/extended_min_max.hpp>

struct BBox {
    glm::vec3 min;
    glm::vec3 max;

    BBox() : min(0.0f), max(0.0f) {}

    BBox(const glm::vec3& min, const glm::vec3& max) : min(min), max(max) {}

    friend bool intersect(const BBox& first, const BBox& second) {
        return first.min.x <= second.max.x && first.max.x >= second.min.x
            && first.min.y <= second.max.y && first.max.y >= second.min.y
            && first.min.z <= second.max.z && first.max.z >= second.min.z;
    }

    friend bool intersect(const BBox& bbox, const glm::vec3& ray_src, const glm::vec3& ray_dir_inverse) {
        const auto dist1 = (bbox.min - ray_src) * ray_dir_inverse;
        const auto dist2 = (bbox.max - ray_src) * ray_dir_inverse;

        const auto dist_min = glm::min(dist1, dist2);
        const auto dist_max = glm::max(dist1, dist2);

        float tmin = glm::max(dist_min.x, dist_min.y, dist_min.z);
        float tmax = glm::min(dist_max.x, dist_max.y, dist_max.z);

        return tmax >= tmin;
    }
};

#endif //SPACEOBJECTS_BBOX_H
