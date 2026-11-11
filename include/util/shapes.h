#pragma once

#include <glm/glm.hpp>

namespace g4d {

enum class shape_type {
    st_circle = 0, 
    st_ellipse = 1, 
    st_line = 2
};

struct circle {
    shape_type type{shape_type::st_circle};
    glm::ivec2 pos;
    int radius;
    color_t color;
}; 

struct ellipse {
    shape_type type{shape_type::st_ellipse};
    glm::ivec2 pos;
    glm::ivec2 paxis;
    color_t color;
};

struct line {
    shape_type type{shape_type::st_line};
    glm::ivec2 pos1;
    glm::ivec2 pos2;
    color_t color;
};

}

