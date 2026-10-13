#pragma once

#include "scene/scene.h"
#include "util/shapes.h"
#include "raster/tree.h"
#include <random>
#include <variant>

namespace g4d {

class tree_scene : public scene {
    std::mt19937 rng;

public:
    tree_scene();
    const char* name() const override { return "playground"; }
    void update(double dt) override;
    void render(frame& f) override;
    void on_mouse_button(int button, int action, int x, int y) override;

    raster_tree<int> rast_tree;
    
};

}