
#include "raster/rasterizer3d.h"
#include "raster/3d/triangle.h"

namespace g4d
{

    screen_vertex clip_to_screen(const clip_vertex &cv, size_t screen_w, size_t screen_h)
    {
        screen_vertex vv;

        float invw = 1.0f / cv.position.w;

        float ndc_x = cv.position.x * invw;
        float ndc_y = cv.position.y * invw;

        float screen_x = (ndc_x + 1.0f) * 0.5f * static_cast<float>(screen_w - 1);
        float screen_y = (ndc_y + 1.0f) * 0.5f * static_cast<float>(screen_h - 1);

        vv.position = glm::vec2(screen_x, screen_y);
        vv.invw = invw;
        vv.normal = cv.normal;
        vv.color = cv.color;

        return vv;
    }

    inline glm::vec3 surface_normal(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3)
    {
        glm::vec3 fn = glm::cross(p2 - p1, p3 - p1);
        float len = glm::length(fn);
        return (len > 1e-6f) ? (fn / len) : glm::vec3(0.0f, 1.0f, 0.0f);
    }

    inline float light_intensity(const glm::vec3 &normal, const glm::vec3 &light_dir, float ambient = 0.25f)
    {
        float diff = std::max(0.0f, glm::dot(normal, light_dir));
        return ambient + (1.0f - ambient) * diff;
    }

    inline color_t apply_light(const color_t &col, float intensity)
    {
        intensity = std::clamp(intensity, 0.0f, 1.0f);
        return color_t(
            static_cast<uint8_t>(std::clamp(col.red * intensity, 0.0f, 255.0f)),
            static_cast<uint8_t>(std::clamp(col.green * intensity, 0.0f, 255.0f)),
            static_cast<uint8_t>(std::clamp(col.blue * intensity, 0.0f, 255.0f)),
            col.alpha
        );
    }

    inline color_t dim(const color_t &col, float amount)
    {
        return apply_light(col, amount);
    }

    rasterizer3d::rasterizer3d(int width, int height)
        : _d_buf(width, height),
          cam(
              glm::vec3{2.0f, 2.0f, 3.0f},
              glm::vec3{-2.0f, -2.0f, -3.0f},
              glm::radians(60.0f),
              static_cast<float>(width) / static_cast<float>(height),
              0.1f, 100.0f)
    {
    }

    size_t rasterizer3d::width() const
    {
        return _d_buf.width();
    }
    size_t rasterizer3d::height() const
    {
        return _d_buf.height();
    }
    void rasterizer3d::resize(size_t width, size_t height)
    {
        _d_buf.resize(width, height);
        cam.set_aspect(static_cast<float>(width) / static_cast<float>(height));
    }
    void rasterizer3d::clear()
    {
        _d_buf.clear();
    }
    void rasterizer3d::raster(frame &f, const object &obj)
    {
        if (_d_buf.width() != f.width() || _d_buf.height() != f.height())
        {
            resize(f.width(), f.height());
            cam.set_aspect(static_cast<float>(f.width()) / static_cast<float>(f.height()));
        }

        glm::mat4 model = obj.get_model();
        glm::mat4 view = cam.get_view_mat();
        glm::mat4 clip = cam.get_clip_mat();
        glm::mat4 mvp = clip * view * model;

        glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(model)));

        glm::vec3 light_dir = glm::normalize(glm::vec3(0.5f, 1.0f, 0.75f));

        size_t num_tris = obj.obj_mesh.num_triangles();

        for (size_t t = 0; t < num_tris; t++)
        {
            size_t i0 = obj.obj_mesh.indices[3 * t];
            size_t i1 = obj.obj_mesh.indices[3 * t + 1];
            size_t i2 = obj.obj_mesh.indices[3 * t + 2];

            const world_vertex &p1 = obj.obj_mesh.vertices[i0];
            const world_vertex &p2 = obj.obj_mesh.vertices[i1];
            const world_vertex &p3 = obj.obj_mesh.vertices[i2];

            glm::vec3 n1, n2, n3;
            if (glm::dot(p1.normal, p1.normal) > 1e-4f)
            {
                n1 = glm::normalize(normal_matrix * p1.normal);
                n2 = glm::normalize(normal_matrix * p2.normal);
                n3 = glm::normalize(normal_matrix * p3.normal);
            }
            else
            {
                glm::vec3 wp1 = glm::vec3(model * glm::vec4(p1.position, 1.0f));
                glm::vec3 wp2 = glm::vec3(model * glm::vec4(p2.position, 1.0f));
                glm::vec3 wp3 = glm::vec3(model * glm::vec4(p3.position, 1.0f));
                glm::vec3 fn = surface_normal(wp1, wp2, wp3);
                n1 = n2 = n3 = fn;
            }

            float intensity1 = light_intensity(n1, light_dir);
            float intensity2 = light_intensity(n2, light_dir);
            float intensity3 = light_intensity(n3, light_dir);

            clip_vertex c1{mvp * glm::vec4(p1.position, 1.0f), n1, apply_light(p1.color, intensity1)};
            clip_vertex c2{mvp * glm::vec4(p2.position, 1.0f), n2, apply_light(p2.color, intensity2)};
            clip_vertex c3{mvp * glm::vec4(p3.position, 1.0f), n3, apply_light(p3.color, intensity3)};

            auto clipped = triangle::clip(c1, c2, c3, cam.get_znear());

            if (clipped.empty())
                continue;

            for (const auto &[ct1, ct2, ct3] : clipped)
            {
                screen_vertex s1 = clip_to_screen(ct1, _d_buf.width(), _d_buf.height());
                screen_vertex s2 = clip_to_screen(ct2, _d_buf.width(), _d_buf.height());
                screen_vertex s3 = clip_to_screen(ct3, _d_buf.width(), _d_buf.height());

                triangle::raster(f, _d_buf, s1, s2, s3);
            }
        }
    }

} // namespace g4d
