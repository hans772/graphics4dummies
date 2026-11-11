#pragma once

#include "scene/scene.h"
#include "raster/bezier.h"
#include "raster/text.h"
#include "raster/conics.h"
#include "raster/line.h"
#include "util/font.h"

namespace g4d {

class bezier_scene : public scene {
public:
    enum class tool_mode {
        add,
        edit,
        del,
        pan
    };

    enum class render_mode {
        subdivision,
        tolerance
    };

    explicit bezier_scene(const std::string_view& fonts_dir);

    const char* name() const override { return "bezier"; }
    void update(double dt) override;
    void render(frame& f) override;
    void on_key(int key, int action) override;
    void on_mouse_move(int x, int y) override;
    void on_mouse_button(int button, int action, int x, int y) override;
    void on_scroll(double xoffset, double yoffset) override;

private:
    font f;
    text_raster tex_r;
    bezier curve;

    tool_mode tool = tool_mode::add;
    render_mode rmode = render_mode::subdivision;

    size_t subdivisions = 30;
    double tolerance = 1.0;

    glm::ivec2 mouse_pos{0, 0};
    glm::ivec2 pan_offset{0, 0};

    bool is_dragging_point = false;
    int selected_point = -1;
    int hovered_point = -1;

    bool is_panning = false;
    glm::ivec2 pan_anchor{0, 0};
    glm::ivec2 pan_base{0, 0};

    glm::vec2 screen_to_world(glm::ivec2 screen_pt) const {
        return glm::vec2(screen_pt - pan_offset);
    }
    glm::ivec2 world_to_screen(glm::vec2 world_pt) const {
        return glm::ivec2(glm::round(world_pt)) + pan_offset;
    }
    int find_near_point(glm::ivec2 screen_pos, float threshold = 16.0f) const;
};

}  // namespace g4d
