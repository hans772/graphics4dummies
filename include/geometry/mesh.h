#pragma once

#include <vector>
#include <string>
#include "geometry/vertex3d.h"
#include "util/off.h"

namespace g4d {

struct mesh {
    std::vector<world_vertex> vertices;
    std::vector<uint32_t> indices; 

    size_t num_triangles() const { return indices.size() / 3; }
};

/**
 * @brief Generates a cube mesh of the given size with distinct face colors and normals.
 * @param size Side length of the cube.
 */
mesh make_cube(float size = 1.0f);

/**
 * @brief Converts an OFF_Mesh (from util/off.h) into a g4d::mesh.
 * @param off_mesh The OFF_Mesh holding raw positions and polygon indices.
 */
mesh from_off(const OFF_Mesh& off_mesh);

inline mesh to_mesh(const OFF_Mesh& off_mesh) {
    return from_off(off_mesh);
}

/**
 * @brief Converts a g4d::mesh into an OFF_Mesh.
 * @param m The mesh to convert.
 */
OFF_Mesh to_off(const mesh& m);

} // namespace g4d