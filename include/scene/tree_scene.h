#pragma once

#include <charconv>
#include <concepts>
#include <string>
#include <string_view>
#include <system_error>

#include "raster/avl_tree.h"
#include "scene/scene.h"
#include "util/shapes.h"
#include "util/color.h"

namespace g4d
{

    /**
     * @brief Any tree derived from raster_tree<V, Node>: raster_tree<V>, avl_tree<V>, ...
     *        The node type is deduced from Tree::node, so only the value type is named.
     *        Use it as `TreeType<int> tree_type`, which means TreeType<tree_type, int>.
     */
    template <typename Tree, typename V>
    concept TreeType = std::derived_from<Tree, raster_tree<V, typename Tree::node>>;

    /**
     * @brief Type a number and press ENTER to insert it into the tree. Press TAB to switch between
     *        insert mode (green prompt) and delete mode (red prompt). BACKSPACE edits the number.
     *
     */
    template <TreeType<int> tree_type>
    class tree_scene : public scene
    {
        static constexpr int toggle_key = GLFW_KEY_TAB;

        font fnt;
        text_raster tex_r;

        glm::ivec2 tree_off{0, 0};
        glm::ivec2 anchor{0, 0};
        glm::ivec2 mouse_pos{0, 0};
        bool anchored{false};

        std::string typed;
        bool delete_mode{false};

    public:
        explicit tree_scene(std::string_view fonts_dir) : fnt(font::load(fonts_dir, "kitchen_sink")), tex_r(fnt)
        {
            tree.config.text = &tex_r;
            tree.config.radius = 30;
            tree.config.label.scale = 2;
            tree.config.y_spacing = 50;
            tree.config.x_spacing = 50;
            tree.config.margin = 200;

            if constexpr (requires { tree.balance_style; })
                tree.balance_style.scale = tree.config.label.scale;
        }

        const char *name() const override { return "tree_scene"; }

        void render(frame &fr) override
        {
            const char *prompt = delete_mode ? "[DELETE NUM]" : "[ENTER NUM]";
            const std::string shown = typed.empty() ? prompt : typed;

            text_style st;
            st.color = delete_mode ? G4D_COLOR_RED : G4D_COLOR_GREEN;
            st.scale = 3;
            const text_extent te = tex_r.measure(shown, st);

            // bottom-right corner
            tex_r.draw(fr, (int)fr.width() - te.width, te.height, shown, st);
            tree.draw(fr, tree_off + (anchored ? (mouse_pos - anchor) : glm::ivec2{0}));
        }

        void on_key(int key, int action) override
        {
            if (action != GLFW_PRESS)
                return;

            if (key >= GLFW_KEY_0 && key <= GLFW_KEY_9)
                typed.push_back(char('0' + key - GLFW_KEY_0));
            else if (key == GLFW_KEY_MINUS)
                typed.push_back('-');
            else if (key == GLFW_KEY_BACKSPACE && !typed.empty())
                typed.pop_back();
            else if (key == toggle_key)
                delete_mode = !delete_mode;
            else if (key == GLFW_KEY_ENTER)
                submit();
        }

        void on_mouse_button(int button, int action, int x, int y) {
            if(action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT && !anchored) {
                anchored = true;
                anchor = mouse_pos = glm::ivec2{x, y};
            } else if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_LEFT && anchored) {
                anchored = false;
                tree_off += (mouse_pos - anchor);
            }
        }

        void on_mouse_move(int x, int y) {
            if(anchored) {
                mouse_pos.x = x;
                mouse_pos.y = y;
            }
        }

        tree_type tree;

    private:
        void submit()
        {
            const char *begin = typed.data();
            const char *end = begin + typed.size();
            int v = 0;
            const auto [ptr, ec] = std::from_chars(begin, end, v);
            if (ec != std::errc{} || ptr != end)
                return;

            if (delete_mode)
                tree.delete_node(v);
            else
                tree.add_node(v);
            typed.clear();
        }
    };

}