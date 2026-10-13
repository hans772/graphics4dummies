#include "scene/tree_scene.h"
#include <iostream>

namespace g4d {

tree_scene::tree_scene(): rng(102929) {}

void tree_scene::update(double dt){}

void tree_scene::render(frame& f) {
    rast_tree.draw(f);
}

void tree_scene::on_mouse_button(int button, int action, int x, int y) {
    if(action == GLFW_PRESS) {
        int v = rng() % 10;
        std::cout << v << std::endl;
        if(rng()%2)rast_tree.add_node(v); 
        else rast_tree.delete_node(v);
        
    }
}

}
