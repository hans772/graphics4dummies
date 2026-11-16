#pragma once

#include <glm/glm.hpp>

namespace g4d
{

class camera {
    glm::vec3 position{0.0f, 0.0f, 3.0f};
    glm::vec3 look_at{0.0f, 0.0f, -1.0f};
    
    float fov{glm::radians(60.0f)};
    float aspect{16.0f / 9.0f};
    float znear{0.1f};
    float zfar{100.0f};

    glm::mat4 view_mat{1.0f};
    glm::mat4 clip_mat{1.0f};

    void update_view_mat();
    void update_clip_mat();

public:
    camera();
    camera(const glm::vec3& pos, const glm::vec3& look, float fov, float aspect, float znear, float zfar);

    const glm::mat4& get_view_mat() const;
    const glm::mat4& get_clip_mat() const;

    glm::vec3 get_position() const;
    void set_position(const glm::vec3& pos);
    void set_look_at(const glm::vec3& look);

    void set_fov(float fov);
    void set_aspect(float aspect);
    void set_znear(float znear);
    void set_zfar(float zfar);

    float get_fov() const;
    float get_aspect() const;
    float get_znear() const;
    float get_zfar() const;

    glm::vec3 looking_at() const;
};
} // namespace g4d
