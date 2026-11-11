// src/bezier.cpp
// Usage: bezier_viewer [shader_dir]
// shader_dir defaults to the assets/shaders folder of the source tree (set by CMake).

#include <cstdio>
#include <exception>
#include <memory>
#include <string>

#include "app/app.h"
#include "scene/bezier.h"

#ifndef G4D_SHADER_DIR
#define G4D_SHADER_DIR "assets/shaders"
#endif

#ifndef G4D_FONTS_DIR
#define G4D_FONTS_DIR "assets/fonts"
#endif

int main(int argc, char** argv) {
    constexpr int W = 1000, H = 700;
    const std::string shaders = argc > 1 ? argv[1] : G4D_SHADER_DIR;

    try {
        g4d::app bezier_viewer(W, H, "Bezier Playground", shaders);
        bezier_viewer.add_scene(std::make_unique<g4d::bezier_scene>(G4D_FONTS_DIR));
        bezier_viewer.run();
    } catch (const std::exception& e) {
        std::fprintf(stderr, "fatal: %s\n", e.what());
        return 1;
    }
}
