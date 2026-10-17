#pragma once

#include <algorithm>
#include <concepts>
#include <string>
#include <utility>

#include "raster/text.h"
#include "raster/tree.h"
#include "util/tree.h"

namespace g4d {

/**
 * @brief A self-balancing (AVL) binary search tree that draws itself.
 *
 * Every node keeps balance = height(right) - height(left), which is always -1, 0 or +1.
 * Insertion and deletion walk back up the path they came down, update the balance factors
 * and rotate where a factor would reach +-2, so lookups, insertions and deletions are all
 * O(log n) and the height stays below ~1.44 * log2(n + 2).
 *
 * When `config.text` is set (see raster_tree::style):
 *  - the node's value is printed inside its circle, and
 *  - its balance factor ("+1", "0" or "-1") is printed above it, shifted to the side
 *    away from the edge to its parent so the edge does not run through the text.
 *
 * Because the height is logarithmic, the recursion in layout and drawing is always shallow.
 */
template <typename T>
requires std::totally_ordered<T>
class avl_tree : public raster_tree<T, node_AVL<T>> {
    using base = raster_tree<T, node_AVL<T>>;

public:
    using node = node_AVL<T>;
    using base::delete_node;   // keeps the delete_node(node*) overload visible

    /** @brief Look of the balance-factor label. */
    text_style balance_style = [] {
        text_style s;
        s.color = 0xff7fd4ff;
        return s;
    }();

    /** @brief Pixels between the top of a node's circle and its balance label. */
    int balance_gap = 2;

    /** @brief Inserts a value and rebalances. Duplicates are ignored. */
    void add_node(T value) override {
        bool grown = false, inserted = false;
        this->head = insert(this->head, value, grown, inserted);
        if (inserted) {
            ++this->count;
            this->dirty = true;
        }
    }

    /** @brief Removes a value if present and rebalances. */
    void delete_node(const T& value) override {
        bool shrunk = false, removed = false;
        this->head = remove(this->head, value, shrunk, removed);
        if (removed) {
            --this->count;
            this->dirty = true;
        }
    }

    /** @brief Height of the tree in nodes (0 for an empty tree). */
    int height() const { return height_of(this->head); }

protected:
    // the root's balance label sticks out radius + gap + glyph height above its centre;
    // make sure that fits inside the frame, whatever margin and radius are configured
    int top_padding() const override {
        if (!this->config.text) return 0;
        const int label_height = this->config.text->get_font().glyph_height() * std::max(1, balance_style.scale);
        const int needed = this->config.radius + balance_gap + label_height;
        return std::max(0, needed - this->config.margin);
    }

    void draw_node(frame& f, const node& n, glm::ivec2 pixel, int side) override {
        base::draw_node(f, n, pixel, side);   // circle + value

        const text_raster* text = this->config.text;
        if (!text) return;

        const std::string s = n.balance > 0 ? "+" + std::to_string(n.balance) : std::to_string(n.balance);
        const text_extent e = text->measure(s, balance_style);

        // above the circle; shifted away from the parent's edge (side: -1 left child, +1 right child)
        const int cx = pixel.x + side * (this->config.radius / 4 + e.width / 2);
        const int cy = pixel.y + this->config.radius + balance_gap + e.height / 2;
        text->draw_centered(f, cx, cy, s, balance_style);
    }

private:
    static int height_of(const node* n) {
        return n ? 1 + std::max(height_of(n->left), height_of(n->right)) : 0;
    }

    // ------------------------------------------------------------------ rotations
    //
    // fix_left_heavy / fix_right_heavy are called when one side of `n` is two levels taller
    // than the other. They rotate, update the balance factors and return the new subtree
    // root. `decreased` reports whether the subtree got shorter overall: always true for the
    // cases insertion can produce, false only for the deletion case where the taller child
    // is itself balanced (a single rotation that leaves the height unchanged).

    // left subtree of n is too tall
    static node* fix_left_heavy(node* n, bool& decreased) {
        node* l = n->left;
        decreased = l->balance != 0;

        if (l->balance <= 0) {            // left-left (or the balanced-child deletion case): single right rotation
            n->left = l->right;
            l->right = n;
            if (l->balance == 0) { n->balance = -1; l->balance = +1; }
            else                 { n->balance = 0;  l->balance = 0; }
            return l;
        }

        node* lr = l->right;              // left-right: rotate l left, then n right
        l->right = lr->left;
        lr->left = l;
        n->left = lr->right;
        lr->right = n;
        n->balance = lr->balance == -1 ? +1 : 0;
        l->balance = lr->balance == +1 ? -1 : 0;
        lr->balance = 0;
        return lr;
    }

    // right subtree of n is too tall (mirror image of fix_left_heavy)
    static node* fix_right_heavy(node* n, bool& decreased) {
        node* r = n->right;
        decreased = r->balance != 0;

        if (r->balance >= 0) {            // right-right (or balanced-child deletion case): single left rotation
            n->right = r->left;
            r->left = n;
            if (r->balance == 0) { n->balance = +1; r->balance = -1; }
            else                 { n->balance = 0;  r->balance = 0; }
            return r;
        }

        node* rl = r->left;               // right-left: rotate r right, then n left
        r->left = rl->right;
        rl->right = r;
        n->right = rl->left;
        rl->left = n;
        n->balance = rl->balance == +1 ? -1 : 0;
        r->balance = rl->balance == -1 ? +1 : 0;
        rl->balance = 0;
        return rl;
    }

    // ------------------------------------------------------------------ insertion
    //
    // Returns the new root of the subtree. `grown` is set when the subtree got one level
    // taller; the caller uses it to update its own balance factor.
    static node* insert(node* n, T& value, bool& grown, bool& inserted) {
        if (!n) {
            grown = inserted = true;
            return new node{std::move(value)};
        }

        if (value < n->value) {
            n->left = insert(n->left, value, grown, inserted);
            if (grown) {
                if (n->balance > 0)       { n->balance = 0; grown = false; }   // was right-heavy: now level
                else if (n->balance == 0) { n->balance = -1; }                 // now left-heavy, still taller
                else { bool unused; n = fix_left_heavy(n, unused); grown = false; }
            }
        } else if (n->value < value) {
            n->right = insert(n->right, value, grown, inserted);
            if (grown) {
                if (n->balance < 0)       { n->balance = 0; grown = false; }
                else if (n->balance == 0) { n->balance = +1; }
                else { bool unused; n = fix_right_heavy(n, unused); grown = false; }
            }
        } else {
            grown = false;   // duplicate
        }
        return n;
    }

    // ------------------------------------------------------------------ deletion
    //
    // `shrunk` is set when the subtree got one level shorter.

    static node* left_shrunk(node* n, bool& shrunk) {    // n's left subtree became shorter
        if (n->balance < 0)       { n->balance = 0; }                     // was left-heavy: level now, but shorter
        else if (n->balance == 0) { n->balance = +1; shrunk = false; }   // height unchanged
        else                      { n = fix_right_heavy(n, shrunk); }
        return n;
    }

    static node* right_shrunk(node* n, bool& shrunk) {   // n's right subtree became shorter
        if (n->balance > 0)       { n->balance = 0; }
        else if (n->balance == 0) { n->balance = -1; shrunk = false; }
        else                      { n = fix_left_heavy(n, shrunk); }
        return n;
    }

    static node* remove(node* n, const T& value, bool& shrunk, bool& removed) {
        if (!n) {
            shrunk = false;
            return nullptr;
        }

        if (value < n->value) {
            n->left = remove(n->left, value, shrunk, removed);
            if (shrunk) n = left_shrunk(n, shrunk);
        } else if (n->value < value) {
            n->right = remove(n->right, value, shrunk, removed);
            if (shrunk) n = right_shrunk(n, shrunk);
        } else {
            removed = true;
            if (!n->left || !n->right) {              // zero or one child: splice the node out
                node* child = n->left ? n->left : n->right;
                delete n;
                shrunk = true;
                return child;
            }
            node* s = n->right;                       // two children: take the in-order successor's value,
            while (s->left) s = s->left;              // then remove that successor from the right subtree
            n->value = s->value;
            bool ignored = false;
            n->right = remove(n->right, n->value, shrunk, ignored);
            if (shrunk) n = right_shrunk(n, shrunk);
        }
        return n;
    }
};

}  // namespace g4d