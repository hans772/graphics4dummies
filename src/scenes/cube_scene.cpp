#include "scene/cube_scene.h"
#include <glm/gtc/quaternion.hpp>
#include <cmath>
#include <string>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace g4d
{

    cube_scene::cube_scene(const std::string_view &fonts_dir)
        : f(font::load(fonts_dir, "kitchen_sink")),
          tex_r(f),
          rasterizer(1000, 700)
    {
        cube_obj.obj_mesh = make_cube(1.2f);
        update_camera();
    }

    void cube_scene::update_camera()
    {
        camera_pitch = std::clamp(camera_pitch, -1.55f, 1.55f);
        float cx = camera_distance * std::cos(camera_pitch) * std::sin(camera_yaw);
        float cy = camera_distance * std::sin(camera_pitch);
        float cz = camera_distance * std::cos(camera_pitch) * std::cos(camera_yaw);

        glm::vec3 pos(cx, cy, cz);
        glm::vec3 look = -pos;
        rasterizer.cam.set_position(pos);
        rasterizer.cam.set_look_at(look);
    }

    void cube_scene::update(double dt)
    {
        time_acc += dt;
        frame_count++;
        if (time_acc >= 0.5)
        {
            current_fps = static_cast<float>(frame_count / time_acc);
            time_acc = 0.0;
            frame_count = 0;
        }

        if (auto_rotate)
        {
            glm::quat q_y = glm::angleAxis(static_cast<float>(dt * 0.85f), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::quat q_x = glm::angleAxis(static_cast<float>(dt * 0.45f), glm::vec3(1.0f, 0.0f, 0.0f));
            cube_obj.rot_rad = q_y * q_x * cube_obj.rot_rad;
        }
    }

    void cube_scene::render(frame &fr)
    {
        rasterizer.clear();
        rasterizer.raster(fr, cube_obj);

        // On-Screen HUD Overlay
        const int top_y = static_cast<int>(fr.height()) - 20;

        text_style text_style_dim{
            .text_color = color_t(200, 200, 200, 255),
            .scale = 1,
            .letter_spacing = 0,
            .line_spacing = 0,
            .background = std::nullopt};

        std::string rot_str = auto_rotate ? "ON" : "OFF";
        tex_r.draw(fr, 15, top_y - 15,
                   "[SPACE] Auto-rotate: " + rot_str +
                       "  |  [LMB Drag] Orbit  |  [Scroll] Zoom  |  [R] Reset",
                   text_style_dim);

        tex_r.draw(fr, 15, top_y - 30,
                   "[1] Colored Cube (Face Normals)  |  [2] Load cube.off | [3] Save cube.off  |  [Arrows] Manual Rotate",
                   text_style_dim);
    }

    void cube_scene::on_key(int key, int action)
    {
        if (action != GLFW_PRESS && action != GLFW_REPEAT)
            return;

        if (action == GLFW_PRESS)
        {
            if (key == GLFW_KEY_SPACE)
            {
                auto_rotate = !auto_rotate;
            }
            else if (key == GLFW_KEY_R)
            {
                cube_obj.rot_rad = glm::quat{1.0f, 0.0f, 0.0f, 0.0f};
                camera_yaw = 0.8f;
                camera_pitch = 0.5f;
                camera_distance = 3.5f;
                update_camera();
            }
            else if (key == GLFW_KEY_1)
            {
                cube_obj.obj_mesh = make_cube(1.2f);
            }
            else if (key == GLFW_KEY_2)
            {
                OFF_Mesh off_data = load_off("bezier_revolve.off");
                if (!off_data.vertices.empty())
                {
                    cube_obj.obj_mesh = from_off(off_data);
                }
            }
            else if (key == GLFW_KEY_3)
            {
                off_write("cube.off", to_off(cube_obj.obj_mesh));
            }
        }

        // Manual rotation via arrow keys
        float rot_delta = 0.1f;
        if (key == GLFW_KEY_LEFT)
        {
            glm::quat q = glm::angleAxis(-rot_delta, glm::vec3(0.0f, 1.0f, 0.0f));
            cube_obj.rot_rad = q * cube_obj.rot_rad;
        }
        else if (key == GLFW_KEY_RIGHT)
        {
            glm::quat q = glm::angleAxis(rot_delta, glm::vec3(0.0f, 1.0f, 0.0f));
            cube_obj.rot_rad = q * cube_obj.rot_rad;
        }
        else if (key == GLFW_KEY_UP)
        {
            glm::quat q = glm::angleAxis(-rot_delta, glm::vec3(1.0f, 0.0f, 0.0f));
            cube_obj.rot_rad = q * cube_obj.rot_rad;
        }
        else if (key == GLFW_KEY_DOWN)
        {
            glm::quat q = glm::angleAxis(rot_delta, glm::vec3(1.0f, 0.0f, 0.0f));
            cube_obj.rot_rad = q * cube_obj.rot_rad;
        }
    }

    void cube_scene::on_mouse_button(int button, int action, int x, int y)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT)
        {
            if (action == GLFW_PRESS)
            {
                mouse_orbiting = true;
                last_mouse = {x, y};
            }
            else if (action == GLFW_RELEASE)
            {
                mouse_orbiting = false;
            }
        }
    }

    void cube_scene::on_mouse_move(int x, int y)
    {
        if (mouse_orbiting)
        {
            int dx = x - last_mouse.x;
            int dy = y - last_mouse.y;
            last_mouse = {x, y};

            camera_yaw -= dx * 0.01f;
            camera_pitch += dy * 0.01f;
            update_camera();
        }
    }

    void cube_scene::on_scroll(double /*xoffset*/, double yoffset)
    {
        camera_distance = std::clamp(camera_distance - static_cast<float>(yoffset) * 0.35f, 1.2f, 15.0f);
        update_camera();
    }

} // namespace g4d
