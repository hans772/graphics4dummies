#include "raster/frame.h"
#include "raster/rasterizer3d.h"
#include "geometry/mesh.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include <iostream>

int main() {
    constexpr int W = 800, H = 600;
    g4d::frame f(W, H);
    g4d::rasterizer3d rasterizer(W, H);

    // Test 1: procedural cube
    g4d::object cube;
    cube.obj_mesh = g4d::make_cube(1.2f);
    rasterizer.clear();
    rasterizer.raster(f, cube);

    // Test 2: loading OFF file via load_off and from_off
    g4d::OFF_Mesh off_data = g4d::load_off("assets/models/cube.off");
    if (off_data.vertices.empty() || off_data.polygons.empty()) {
        std::cerr << "Failed to load OFF file!\n";
        return 1;
    }
    if (off_data.colors.size() != 12) {
        std::cerr << "Expected 12 face colors from cube.off, got " << off_data.colors.size() << "\n";
        return 1;
    }
    g4d::mesh converted = g4d::from_off(off_data);
    if (converted.num_triangles() != 12) {
        std::cerr << "Unexpected converted mesh size: " << converted.vertices.size() 
                  << " vertices, " << converted.num_triangles() << " triangles\n";
        return 1;
    }

    std::cout << "Successfully verified load_off and from_off conversion:\n"
              << "  OFF vertices: " << off_data.vertices.size() 
              << ", polygons: " << off_data.polygons.size() 
              << ", face colors: " << off_data.colors.size() << "\n"
              << "  Mesh vertices: " << converted.vertices.size() 
              << ", triangles: " << converted.num_triangles() << "\n";

    // Test 3: render the loaded OFF mesh with face colors to ensure it renders correctly
    g4d::object off_cube_obj;
    off_cube_obj.obj_mesh = std::move(converted);
    rasterizer.clear();
    rasterizer.raster(f, off_cube_obj);

    // Test 4: verify writing OFF file with colors
    g4d::off_write("cube_saved.off", off_data);
    g4d::OFF_Mesh reloaded = g4d::load_off("cube_saved.off");
    if (reloaded.colors.size() != 12 || reloaded.vertices.size() != off_data.vertices.size()) {
        std::cerr << "Failed to round-trip OFF file with colors!\n";
        return 1;
    }
    std::cout << "Successfully verified off_write and load_off color round-trip (12 triangle colors preserved).\n";

    // Test 5: verify to_off (converting mesh to OFF_Mesh)
    g4d::OFF_Mesh off_from_cube = g4d::to_off(cube.obj_mesh);
    if (off_from_cube.vertices.size() != 24 || off_from_cube.polygons.size() != 12 || off_from_cube.colors.size() != 12) {
        std::cerr << "Failed to convert mesh to OFF_Mesh via to_off!\n";
        return 1;
    }
    std::cout << "Successfully verified to_off (converted mesh to OFF_Mesh with "
              << off_from_cube.vertices.size() << " vertices and " 
              << off_from_cube.polygons.size() << " triangles).\n";

    int ret = stbi_write_png("cube_render.png", W, H, 4, f.get_buffer(), W * 4);
    if (ret != 0) {
        std::cout << "Successfully rendered cube to cube_render.png (" << W << "x" << H << ")\n";
        return 0;
    }
    return 1;
}
