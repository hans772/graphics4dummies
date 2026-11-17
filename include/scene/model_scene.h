#pragma once

#include "scene/scene.h"
#include "raster/rasterizer3d.h"
#include "object/object.h"
#include "raster/2d/text.h"
#include "util/font.h"

#include <string_view>

namespace g4d {

class model_scene : public scene {
public:
    explicit model_scene(const std::string_view& fonts_dir, const std::string_view& model_path);

    const char* name() const override { return "3D Model Rasterizer"; }

    void update(double dt) override;
    void render(frame& f) override;

    void on_key(int key, int action) override;
    void on_mouse_button(int button, int action, int x, int y) override;
    void on_mouse_move(int x, int y) override;
    void on_scroll(double xoffset, double yoffset) override;

    void update_camera(double dt);
    void update_camera_orientation();
    void reset_camera();

private:

    font f;
    text_raster tex_r;

    rasterizer3d rasterizer;
    object obj;

    std::string model_path;
    bool loaded = false;

    bool mouse_orbiting = false;
    glm::ivec2 last_mouse{0, 0};

    float camera_yaw = 0.8f;
    float camera_pitch = -0.5f;
    float cam_speed = 3.0f;

    bool key_w = false;
    bool key_s = false;
    bool key_a = false;
    bool key_d = false;
    bool key_left = false;
    bool key_right = false;
    bool key_up = false;
    bool key_down = false;

    float current_fps = 0.0f;
    double time_acc = 0.0;
    int frame_count = 0;
};

} // namespace g4d
