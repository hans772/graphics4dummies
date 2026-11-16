#pragma once

#include "scene/scene.h"
#include "raster/rasterizer3d.h"
#include "object/object.h"
#include "raster/2d/text.h"
#include "util/font.h"

#include <string_view>

namespace g4d {

class cube_scene : public scene {
public:
    explicit cube_scene(const std::string_view& fonts_dir);

    const char* name() const override { return "3D Cube Rasterizer"; }

    void update(double dt) override;
    void render(frame& f) override;

    void on_key(int key, int action) override;
    void on_mouse_button(int button, int action, int x, int y) override;
    void on_mouse_move(int x, int y) override;
    void on_scroll(double xoffset, double yoffset) override;

private:
    void update_camera();

    font f;
    text_raster tex_r;

    rasterizer3d rasterizer;
    object cube_obj;

    bool auto_rotate = true;
    bool mouse_orbiting = false;
    glm::ivec2 last_mouse{0, 0};

    float camera_yaw = 0.8f;
    float camera_pitch = 0.5f;
    float camera_distance = 20.5f;

    float current_fps = 0.0f;
    double time_acc = 0.0;
    int frame_count = 0;
};

} // namespace g4d
