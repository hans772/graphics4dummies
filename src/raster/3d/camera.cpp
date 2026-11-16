#include "raster/3d/camera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace g4d
{

    void camera::update_view_mat() {
        view_mat = glm::lookAt(position, position + look_at, glm::vec3(0.0f, 1.0f, 0.0f));
    }

    void camera::update_clip_mat() {
        clip_mat = glm::perspective(fov, aspect, znear, zfar);
    }

    camera::camera() {
        update_clip_mat();
        update_view_mat();
    }

    camera::camera(const glm::vec3& pos, const glm::vec3& look, float f, float a, float zn, float zf) : 
        position(pos), 
        look_at(look), 
        fov(f), 
        aspect(a), 
        znear(zn), 
        zfar(zf) 
    {
        update_clip_mat();
        update_view_mat();
    }

    const glm::mat4& camera::get_view_mat() const {
        return view_mat;
    }

    const glm::mat4& camera::get_clip_mat() const {
        return clip_mat;
    }

    glm::vec3 camera::get_position() const {
        return position;
    }

    void camera::set_position(const glm::vec3& pos) {
        position = pos;
        update_view_mat();
    }
    void camera::set_look_at(const glm::vec3& pos) {
        look_at = pos;
        update_view_mat();
    }

    void camera::set_fov(float f) {
        fov = f;
        update_clip_mat();
    }
    void camera::set_aspect(float a) {
        aspect = a;
        update_clip_mat();
    }

    void camera::set_znear(float zn) {
        znear = zn;
        update_clip_mat();
    }

    void camera::set_zfar(float zf) {
        zfar = zf;
        update_clip_mat();
    }
    
    float camera::get_fov() const { return fov; }
    float camera::get_aspect() const { return aspect; }
    glm::vec3 camera::looking_at() const { return look_at; }
    float camera::get_znear() const { return znear; }
    float camera::get_zfar() const { return zfar; }
} // namespace g4d
