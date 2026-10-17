#pragma once

#include <concepts>
#include <utility>

#include <glm/glm.hpp>

namespace g4d {

/** @brief Plain binary search tree node. */
template <typename T>
struct tree_node {

    T value;

    glm::ivec2 position{0};
    tree_node* left{nullptr};
    tree_node* right{nullptr};

    int modifier{0};

};

/**
 * @brief AVL tree node: a tree_node plus a balance factor.
 *
 * balance = height(right subtree) - height(left subtree), so +1 means right-heavy,
 * -1 left-heavy and 0 balanced. In a valid AVL tree it is always -1, 0 or +1.
 *
 * This is deliberately NOT derived from tree_node<T>. A derived node would inherit
 * `tree_node<T>* left/right`, so `n->left->balance` would not compile and every child
 * access in the tree code would need a cast. With its own `node_AVL*` children, the same
 * generic tree code works for both node types.
 */
template <typename T>
struct node_AVL {

    T value;

    glm::ivec2 position{0};
    node_AVL* left{nullptr};
    node_AVL* right{nullptr};

    int modifier{0};

    int balance{0};

};

/**
 * @brief What raster_tree needs from a node type N holding values of type T:
 *        `value`, `position` (with int x and y), `modifier`, children `left` and `right`
 *        that are `N*`, and construction from a value with `N{value}`.
 */
template <typename N, typename T>
concept tree_node_type = requires(N n) {
    { n.value } -> std::same_as<T&>;
    { n.position.x } -> std::convertible_to<int>;
    { n.position.y } -> std::convertible_to<int>;
    { n.modifier } -> std::same_as<int&>;
    { n.left } -> std::same_as<N*&>;
    { n.right } -> std::same_as<N*&>;
    N{std::declval<T>()};
};

}