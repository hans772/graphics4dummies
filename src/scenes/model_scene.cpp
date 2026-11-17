#include "scene/model_scene.h"
#include "geometry/mesh.h"
#include <glm/gtc/quaternion.hpp>
#include <cmath>
#include <string>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace g4d
{

    model_scene::model_scene(const std::string_view &fonts_dir, const std::string_view& path)
        : f(font::load(fonts_dir, "kitchen_sink")),
          tex_r(f),
          rasterizer(1000, 700),
          model_path(path)
    {
        OFF_Mesh off_data = load_off(model_path);
        if (!off_data.vertices.empty())
        {
            obj.obj_mesh = mesh::from_off(off_data);
            loaded = true;
        } else obj.obj_mesh = mesh::make_cube(1.2f);

        reset_camera();
    }

    void model_scene::reset_camera()
    {
        constexpr float dist = 3.5f;
        constexpr float elev = 0.5f;
        camera_yaw = 0.8f;
        camera_pitch = -elev;

        float cx = dist * std::cos(elev) * std::sin(camera_yaw);
        float cy = dist * std::sin(elev);
        float cz = dist * std::cos(elev) * std::cos(camera_yaw);

        rasterizer.cam.set_position(glm::vec3(cx, cy, cz));
        update_camera_orientation();
    }

    void model_scene::update_camera_orientation()
    {
        constexpr float pitch_limit = glm::radians(89.0f);
        camera_pitch = std::clamp(camera_pitch, -pitch_limit, pitch_limit);

        glm::vec3 front;
        front.x = -std::cos(camera_pitch) * std::sin(camera_yaw);
        front.y = std::sin(camera_pitch);
        front.z = -std::cos(camera_pitch) * std::cos(camera_yaw);

        rasterizer.cam.look_in(glm::normalize(front));
    }

    void model_scene::update_camera(double dt)
    {
        constexpr float rot_speed = 1.5f;
        bool rot_changed = false;

        if (key_left)
        {
            camera_yaw += rot_speed * static_cast<float>(dt);
            rot_changed = true;
        }
        if (key_right)
        {
            camera_yaw -= rot_speed * static_cast<float>(dt);
            rot_changed = true;
        }
        if (key_up)
        {
            camera_pitch += rot_speed * static_cast<float>(dt);
            rot_changed = true;
        }
        if (key_down)
        {
            camera_pitch -= rot_speed * static_cast<float>(dt);
            rot_changed = true;
        }

        if (rot_changed)
        {
            update_camera_orientation();
        }

        int move_f = (key_w ? 1 : 0) - (key_s ? 1 : 0);
        int move_s = (key_d ? 1 : 0) - (key_a ? 1 : 0);
        glm::vec3 right = glm::normalize(glm::cross(rasterizer.cam.looking_in(), rasterizer.cam.get_up()));
        if (move_s != 0 || move_f != 0)
        {
            rasterizer.cam.set_position(
                rasterizer.cam.get_position() +
                (rasterizer.cam.looking_in() * (cam_speed * static_cast<float>(dt * move_f))) +
                right * (cam_speed * static_cast<float>(dt * move_s))
            );
        }
    }

    void model_scene::update(double dt)
    {
        time_acc += dt;
        frame_count++;
        if (time_acc >= 0.5)
        {
            current_fps = static_cast<float>(frame_count / time_acc);
            time_acc = 0.0;
            frame_count = 0;
        }

        update_camera(dt);
    }

    void model_scene::render(frame &fr)
    {
        rasterizer.clear();
        rasterizer.raster(fr, obj);
    }

    void model_scene::on_key(int key, int action)
    {
        if (action == GLFW_PRESS)
        {
            if (key == GLFW_KEY_R)
            {
                obj.rot = glm::quat{1.0f, 0.0f, 0.0f, 0.0f};
                obj.position = glm::vec3{0.0f, 0.0f, 0.0f};
                reset_camera();
            }
        }

        bool is_down = (action != GLFW_RELEASE);

        if (key == GLFW_KEY_W) key_w = is_down;
        else if (key == GLFW_KEY_S) key_s = is_down;
        else if (key == GLFW_KEY_A) key_a = is_down;
        else if (key == GLFW_KEY_D) key_d = is_down;
        else if (key == GLFW_KEY_LEFT)key_left = is_down;
        else if (key == GLFW_KEY_RIGHT)key_right = is_down;
        else if (key == GLFW_KEY_UP) key_up = is_down;
        else if (key == GLFW_KEY_DOWN)key_down = is_down;
    }

    void model_scene::on_mouse_button(int button, int action, int x, int y)
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

    void model_scene::on_mouse_move(int x, int y)
    {
        if (mouse_orbiting)
        {
            int dx = x - last_mouse.x;
            int dy = y - last_mouse.y;
            last_mouse = {x, y};

            camera_yaw -= static_cast<float>(dx) * 0.005f;
            camera_pitch -= static_cast<float>(dy) * 0.005f;
            update_camera_orientation();
        }
    }

    void model_scene::on_scroll(double /*xoffset*/, double yoffset)
    {
        cam_speed = std::clamp(cam_speed + static_cast<float>(yoffset) * 0.5f, 0.5f, 20.0f);
    }

} // namespace g4d
