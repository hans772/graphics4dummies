#pragma once

#include <vector>
#include <string>
#include "geometry/vertex3d.h"
#include "util/off.h"

namespace g4d {

struct mesh_t {
    std::vector<world_vertex> vertices;
    std::vector<uint32_t> indices; 

    size_t num_triangles() const { return indices.size() / 3; }
};

namespace mesh {
    /**
     * @brief Generates a cube mesh of the given size with distinct face colors and normals.
     * @param size Side length of the cube.
     */
    mesh_t make_cube(float size = 1.0f);

    /**
     * @brief Converts an OFF_Mesh (from util/off.h) into a g4d::mesh.
     * @param off_mesh The OFF_Mesh holding raw positions and polygon indices.
     */
    mesh_t from_off(const OFF_Mesh& off_mesh);

    inline mesh_t to_mesh(const OFF_Mesh& off_mesh) {
        return from_off(off_mesh);
    }

    /**
     * @brief Converts a g4d::mesh into an OFF_Mesh.
     * @param m The mesh to convert.
     */
    OFF_Mesh to_off(const mesh_t& m);
}


} // namespace g4d