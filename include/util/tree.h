#pragma once

#include <glm/glm.hpp>

namespace g4d {

template <typename T>
struct tree_node {

    T value;

    glm::ivec2 position{0};
    tree_node* left{nullptr};
    tree_node* right{nullptr};

    int modifier{0};

};

}