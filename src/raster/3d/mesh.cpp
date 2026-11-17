#include "geometry/mesh.h"
#include <algorithm>
#include <array>

namespace g4d {

mesh_t mesh::make_cube(float size) {
    mesh_t m;
    float h = size * 0.5f;

    color_t c_front(235, 65, 65);
    color_t c_back(50, 200, 220);
    color_t c_right(65, 120, 240);
    color_t c_left(240, 195, 50);
    color_t c_top(65, 215, 80);
    color_t c_bottom(215, 65, 215);

    m.vertices = {

        { glm::vec3(-h, -h,  h), glm::vec3( 0.0f,  0.0f,  1.0f), c_front },
        { glm::vec3( h, -h,  h), glm::vec3( 0.0f,  0.0f,  1.0f), c_front },
        { glm::vec3( h,  h,  h), glm::vec3( 0.0f,  0.0f,  1.0f), c_front },
        { glm::vec3(-h,  h,  h), glm::vec3( 0.0f,  0.0f,  1.0f), c_front },

        { glm::vec3( h, -h, -h), glm::vec3( 0.0f,  0.0f, -1.0f), c_back },
        { glm::vec3(-h, -h, -h), glm::vec3( 0.0f,  0.0f, -1.0f), c_back },
        { glm::vec3(-h,  h, -h), glm::vec3( 0.0f,  0.0f, -1.0f), c_back },
        { glm::vec3( h,  h, -h), glm::vec3( 0.0f,  0.0f, -1.0f), c_back },

        { glm::vec3( h, -h,  h), glm::vec3( 1.0f,  0.0f,  0.0f), c_right },
        { glm::vec3( h, -h, -h), glm::vec3( 1.0f,  0.0f,  0.0f), c_right },
        { glm::vec3( h,  h, -h), glm::vec3( 1.0f,  0.0f,  0.0f), c_right },
        { glm::vec3( h,  h,  h), glm::vec3( 1.0f,  0.0f,  0.0f), c_right },

        { glm::vec3(-h, -h, -h), glm::vec3(-1.0f,  0.0f,  0.0f), c_left },
        { glm::vec3(-h, -h,  h), glm::vec3(-1.0f,  0.0f,  0.0f), c_left },
        { glm::vec3(-h,  h,  h), glm::vec3(-1.0f,  0.0f,  0.0f), c_left },
        { glm::vec3(-h,  h, -h), glm::vec3(-1.0f,  0.0f,  0.0f), c_left },

        { glm::vec3(-h,  h,  h), glm::vec3( 0.0f,  1.0f,  0.0f), c_top },
        { glm::vec3( h,  h,  h), glm::vec3( 0.0f,  1.0f,  0.0f), c_top },
        { glm::vec3( h,  h, -h), glm::vec3( 0.0f,  1.0f,  0.0f), c_top },
        { glm::vec3(-h,  h, -h), glm::vec3( 0.0f,  1.0f,  0.0f), c_top },

        { glm::vec3(-h, -h, -h), glm::vec3( 0.0f, -1.0f,  0.0f), c_bottom },
        { glm::vec3( h, -h, -h), glm::vec3( 0.0f, -1.0f,  0.0f), c_bottom },
        { glm::vec3( h, -h,  h), glm::vec3( 0.0f, -1.0f,  0.0f), c_bottom },
        { glm::vec3(-h, -h,  h), glm::vec3( 0.0f, -1.0f,  0.0f), c_bottom }
    };

    // 12 triangles (2 per face), CCW winding
    m.indices.reserve(36);
    for (uint32_t face = 0; face < 6; ++face) {
        uint32_t base = face * 4;
        m.indices.push_back(base + 0);
        m.indices.push_back(base + 1);
        m.indices.push_back(base + 2);

        m.indices.push_back(base + 0);
        m.indices.push_back(base + 2);
        m.indices.push_back(base + 3);
    }

    return m;
}

mesh_t mesh::from_off(const OFF_Mesh& off_mesh) {
    mesh_t m;

    if (!off_mesh.colors.empty()) {
        m.vertices.reserve(off_mesh.polygons.size() * 3);
        m.indices.reserve(off_mesh.polygons.size() * 3);

        for (size_t p = 0; p < off_mesh.polygons.size(); ++p) {
            const auto& poly = off_mesh.polygons[p];
            if (poly.size() < 3) continue;

            color_t c = (p < off_mesh.colors.size()) ? off_mesh.colors[p] : color_t(200);
            uint32_t base = static_cast<uint32_t>(m.vertices.size());

            for (size_t i = 0; i < 3; ++i) {
                const auto& v = off_mesh.vertices[poly[i]];
                m.vertices.push_back({ glm::vec4(v[0], v[1], v[2], 1.0f), glm::vec3(0.0f), c });
                m.indices.push_back(base + static_cast<uint32_t>(i));
            }
        }
        return m;
    }

    m.vertices.reserve(off_mesh.vertices.size());
    for (size_t i = 0; i < off_mesh.vertices.size(); ++i) {
        color_t c = (i < off_mesh.vertex_colors.size()) ? off_mesh.vertex_colors[i] : color_t(200);
        const auto& v = off_mesh.vertices[i];
        m.vertices.push_back({ glm::vec4(v[0], v[1], v[2], 1.0f), glm::vec3(0.0f), c });
    }

    m.indices.reserve(off_mesh.polygons.size() * 3);
    for (const auto& poly : off_mesh.polygons) {
        if (poly.size() < 3) continue;
        m.indices.push_back(static_cast<uint32_t>(poly[0]));
        m.indices.push_back(static_cast<uint32_t>(poly[1]));
        m.indices.push_back(static_cast<uint32_t>(poly[2]));
    }

    return m;
}

OFF_Mesh mesh::to_off(const mesh_t& m) {
    OFF_Mesh off;
    off.vertices.reserve(m.vertices.size());
    off.vertex_colors.reserve(m.vertices.size());

    for (const auto& v : m.vertices) {
        off.vertices.push_back({
            static_cast<double>(v.position.x),
            static_cast<double>(v.position.y),
            static_cast<double>(v.position.z)
        });
        off.vertex_colors.push_back(v.color);
    }

    size_t num_tris = m.num_triangles();
    off.polygons.reserve(num_tris);
    off.colors.reserve(num_tris);

    for (size_t t = 0; t < num_tris; ++t) {
        size_t i0 = m.indices[3 * t];
        size_t i1 = m.indices[3 * t + 1];
        size_t i2 = m.indices[3 * t + 2];
        off.polygons.push_back({i0, i1, i2});

        if (i0 < m.vertices.size()) {
            off.colors.push_back(m.vertices[i0].color);
        }
    }

    return off;
}

} // namespace g4d
