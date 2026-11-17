// src/cube.cpp
// Usage: cube_viewer [shader_dir]
// shader_dir defaults to the assets/shaders folder of the source tree (set by CMake).

#include <cstdio>
#include <exception>
#include <memory>
#include <string>

#include "app/app.h"
#include "scene/model_scene.h"

#ifndef G4D_SHADER_DIR
#define G4D_SHADER_DIR "assets/shaders"
#endif

#ifndef G4D_FONTS_DIR
#define G4D_FONTS_DIR "assets/fonts"
#endif

int main(int argc, char** argv) {
    if(argc < 2) {
        std::printf("usage: %s path/to/model [path/to/shader/dir]\n", argv[0]);
        return 0;
    }
    constexpr int W = 1000, H = 700;
    const std::string shaders = argc > 2 ? argv[2] : G4D_SHADER_DIR;

    try {
        g4d::app model_viewer(W, H, "3D Model Rasterizer", shaders);
        model_viewer.add_scene(std::make_unique<g4d::model_scene>(G4D_FONTS_DIR, argv[1]));
        model_viewer.run();
    } catch (const std::exception& e) {
        std::fprintf(stderr, "fatal: %s\n", e.what());
        return 1;
    }
}
