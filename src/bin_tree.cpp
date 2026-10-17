// src/app/main.cpp
// Usage: viewer [shader_dir]
// shader_dir defaults to the assets/shaders folder of the source tree (set by CMake).

#include <cstdio>
#include <exception>
#include <memory>
#include <string>

#include "app/app.h"
#include "scene/tree_scene.h"
#include "raster/avl_tree.h"
#include "raster/tree.h"

#ifndef G4D_SHADER_DIR
#define G4D_SHADER_DIR "assets/shaders"
#endif
#ifndef G4D_FONTS_DIR
#define G4D_FONTS_DIR "assets/fonts"
#endif

int main(int argc, char** argv) {
    constexpr int W = 1200, H = 800;
    const std::string shaders = argc > 1 ? argv[1] : G4D_SHADER_DIR;

    try {
        g4d::app tree_viewer(W, H, "Binary Tree", shaders);
        tree_viewer.add_scene(std::make_unique<g4d::tree_scene<g4d::raster_tree<int>>>(G4D_FONTS_DIR));
        tree_viewer.run();
    } catch (const std::exception& e) {
        std::fprintf(stderr, "fatal: %s\n", e.what());
        return 1;
    }
}