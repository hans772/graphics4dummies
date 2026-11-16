#pragma once

#include <glm/glm.hpp>
#include "util/color.h"

namespace g4d
{
    struct world_vertex {
        glm::vec3 position{0.0f, 0.0f, 0.0f};
        glm::vec3 normal{0.0f};
        color_t color{255};
    };

    struct clip_vertex {
        glm::vec4 position{0.0f, 0.0f, 0.0f, 1.0f};
        glm::vec3 normal{0.0f};
        color_t color{255};
    };

    struct screen_vertex {
        glm::vec2 position{0.0f};
        glm::vec3 normal{0.0f};
        float invw{1.0f};
        color_t color{255};
    };

} // namespace g4d
