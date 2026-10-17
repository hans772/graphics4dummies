#pragma once

#include <algorithm>
#include <climits>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <glm/glm.hpp>

#include "raster/frame.h"
#include "raster/text.h"
#include "util/tree.h"

namespace g4d {

namespace detail {

/** @brief Default node label: the value printed with operator<<, or "" if T cannot be printed. */
template <typename V>
std::string default_label(const V& v) {
    if constexpr (requires(std::ostringstream& os, const V& x) { os << x; }) {
        std::ostringstream os;
        os << v;
        return os.str();
    } else {
        return {};
    }
}

}  // namespace detail

/**
 * @brief A binary search tree that lays itself out and draws itself into a g4d::frame.
 *
 * Layout is the level-by-level algorithm from the paper (two recursive passes, see
 * modify() and place() instead of the iterative approach by Shannon and Wetherell).
 *
 * @tparam T     value type, must be totally ordered
 * @tparam Node  node type, see g4d::tree_node_type (tree_node<T> or node_AVL<T>, or your own)
 *
 * Node labels: set `config.text` to a text_raster and every node shows its value, centred
 * inside the circle. Derived classes can add more through draw_node().
 *
 * Needs g4d::frame to provide `width()` and `height()` (const) accessors.
 *
 * @warning The layout and the plain BST removal are recursive: a degenerate tree with ~100k
 *          nodes can overflow the stack. (Balanced trees such as avl_tree are not affected.)
 */
template <typename T, typename Node = tree_node<T>>
requires std::totally_ordered<T> && tree_node_type<Node, T>
class raster_tree {
public:
    using node = Node;

    /** @brief Pixel-space look of the drawing. */
    struct style {
        int x_spacing = 20;   ///< pixels per horizontal layout unit (neighbours are >= 2 units apart)
        int y_spacing = 20;   ///< pixels per vertical layout unit (one tree level = 2 units)
        int radius = 10;      ///< node circle radius in pixels
        int margin = 20;      ///< pixels between the drawing and the frame border
        uint32_t node_color = 0xffffffff;   ///< 0xAARRGGBB
        uint32_t edge_color = 0xff888888;

        /** Text renderer for labels; nullptr = no labels. Must outlive the tree. */
        const text_raster* text = nullptr;
        /** Look of the value label drawn inside each node. */
        text_style label;
        /** Turns a value into its label. The default prints it with operator<<. */
        std::function<std::string(const T&)> to_string = [](const T& v) { return detail::default_label(v); };
    };

    style config;

    raster_tree() = default;
    virtual ~raster_tree() { destroy(head); }

    raster_tree(const raster_tree&) = delete;
    raster_tree& operator=(const raster_tree&) = delete;

    /** @brief Inserts a value into the tree. Duplicates are ignored. */
    virtual void add_node(T value) {
        node** link = &head;
        while (*link) {
            if (value < (*link)->value)      link = &(*link)->left;
            else if ((*link)->value < value) link = &(*link)->right;
            else return;
        }
        *link = new node{std::move(value)};
        ++count;
        dirty = true;
    }

    /** @brief Removes a value if present. */
    virtual void delete_node(const T& value) {
        bool removed = false;
        head = remove(head, value, removed);
        if (removed) {
            --count;
            dirty = true;
        }
    }

    /** @brief Removes the node a pointer refers to (looked up again by value). */
    void delete_node(node* n) {
        if (n) delete_node(n->value);
    }

    /** @brief Computes node positions and sets their positions accordingly */
    void structure() {
        min_x = INT_MAX;
        structure_data data;
        modify(head, 0, data);
        place(head, 0, 0);
        if (!head) min_x = 0;
        dirty = false;
    }

    /** @brief Draws edges, then nodes on top. */
    virtual void draw(frame& f) {
        if (!head) return;
        if (dirty) structure();
        draw_edges(f, head);
        draw_nodes(f, head, 0);
    }

    const node* root() const { return head; }
    size_t size() const { return count; }

    bool contains(const T& value) const {
        const node* n = head;
        while (n) {
            if (value < n->value)      n = n->left;
            else if (n->value < value) n = n->right;
            else return true;
        }
        return false;
    }

protected:
    /**
     * @brief Draws one node: the circle, plus its value if `config.text` is set.
     *        Override to colour by depth, add more labels, etc.
     *
     * @param pixel  centre of the node
     * @param side   -1 if the node is the left child of its parent, +1 if it is the right
     *               child, 0 for the root. The edge to the parent leaves the node towards
     *               the opposite side, so labels can use it to stay clear of that edge.
     */
    virtual void draw_node(frame& f, const node& n, glm::ivec2 pixel, int /*side*/) {
        f.draw_circle(pixel.x, pixel.y, (size_t)config.radius, config.node_color);
        if (config.text && config.to_string) {
            const std::string s = config.to_string(n.value);
            if (!s.empty()) config.text->draw_centered(f, pixel.x, pixel.y, s, config.label);
        }
    }

    /**
     * @brief Extra space (pixels) to keep free above the root, on top of `config.margin`.
     *        Derived classes that draw labels above the nodes return what they need so the
     *        root's label is not cut off by the top of the frame.
     */
    virtual int top_padding() const { return 0; }

    /** @brief Layout position -> pixel (origin bottom-left, y up; the root is at the top). */
    glm::ivec2 to_pixel(const node& n, const frame& f) const {
        return {config.margin + (n.position.x - min_x) * config.x_spacing,
                (int)f.height() - config.margin - top_padding() - (n.position.y - 1) * config.y_spacing};
    }

    node* head = nullptr;
    size_t count = 0;
    bool dirty = false;

private:

    struct structure_data {
        std::vector<int> modifiers;
        std::vector<int> next_pos;
    };

    // Pass 1 (post-order): provisional x for every node.
    void modify(node* n, int depth, structure_data& d) {
        if (!n) return;
        if (depth == (int)d.modifiers.size()) {
            d.modifiers.push_back(0);
            d.next_pos.push_back(1);
        }

        modify(n->left, depth + 1, d);
        modify(n->right, depth + 1, d);

        const bool leaf = !n->left && !n->right;
        int ideal;
        if (n->left && n->right) ideal = (n->left->position.x + n->right->position.x) / 2;
        else if (n->left)        ideal = n->left->position.x + 1;
        else if (n->right)       ideal = n->right->position.x - 1;
        else                     ideal = d.next_pos[depth];

        int& mod = d.modifiers[depth];
        mod = std::max(mod, d.next_pos[depth] - ideal);

        n->position.x = leaf ? ideal : ideal + mod;
        n->modifier = mod;
        d.next_pos[depth] = n->position.x + 2;
    }

    // Pass 2 (pre-order): apply the accumulated shifts and assign y.
    void place(node* n, int depth, int shift) {
        if (!n) return;
        n->position.x += shift;
        n->position.y = 2 * depth + 1;
        min_x = std::min(min_x, n->position.x);
        place(n->left, depth + 1, shift + n->modifier);
        place(n->right, depth + 1, shift + n->modifier);
    }

    void draw_edges(frame& f, const node* n) {
        const glm::dvec2 a = glm::dvec2(to_pixel(*n, f));
        for (const node* c : {n->left, n->right}) {
            if (!c) continue;
            const glm::dvec2 b = glm::dvec2(to_pixel(*c, f));
            const glm::dvec2 dir = glm::normalize(b - a); //direction to child
            const glm::dvec2 p = a + dir * (double)config.radius;
            const glm::dvec2 q = b - dir * (double)config.radius;
            f.draw_line_coh_sut((int)std::lround(p.x), (int)std::lround(p.y),
                                (int)std::lround(q.x), (int)std::lround(q.y), config.edge_color);
            draw_edges(f, c);
        }
    }

    void draw_nodes(frame& f, const node* n, int side) {
        if (!n) return;
        draw_node(f, *n, to_pixel(*n, f), side);
        draw_nodes(f, n->left, -1);
        draw_nodes(f, n->right, +1);
    }

    static node* remove(node* n, const T& value, bool& removed) {
        if (!n) return nullptr;
        if (value < n->value) {
            n->left = remove(n->left, value, removed);
        } else if (n->value < value) {
            n->right = remove(n->right, value, removed);
        } else {
            removed = true;
            if (!n->left)  { node* r = n->right; delete n; return r; }
            if (!n->right) { node* l = n->left;  delete n; return l; }
            node* s = n->right;
            while (s->left) s = s->left;
            n->value = s->value;
            bool ignored = false;
            n->right = remove(n->right, n->value, ignored);

        }
        return n;
    }

    static void destroy(node* root) {
        std::vector<node*> stack;
        if (root) stack.push_back(root);
        while (!stack.empty()) {
            node* n = stack.back();
            stack.pop_back();
            if (n->left) stack.push_back(n->left);
            if (n->right) stack.push_back(n->right);
            delete n;
        }
    }

    int min_x = 0;
};

}  // namespace g4d