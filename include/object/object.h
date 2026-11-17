#pragma once

#include "geometry/mesh.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

namespace g4d{   

struct object {
    mesh_t obj_mesh;
    glm::vec3 position{0.0f};
    glm::quat rot{1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 scale{1.0f};

    glm::mat4 get_model() const {
        glm::mat4 transl{glm::translate(glm::mat4{1.0f}, position)};
        glm::mat4 rotate{glm::mat4_cast(rot)};
        glm::mat4 _scale{glm::scale(glm::mat4{1.0f}, scale)};

        return transl * rotate * _scale;
    }
};

} // namespace g4d
