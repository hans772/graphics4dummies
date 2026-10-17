#include "scene/playground.h"
#include "raster/text.h"
#include <iostream>

template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

g4d::shape_type get_shape(int i) {
    switch(i) {
        case 0: return g4d::shape_type::st_circle;
        case 1: return g4d::shape_type::st_ellipse;
        case 2: return g4d::shape_type::st_line;
        default: return g4d::shape_type::st_line;
    }
}

std::string shape_name(g4d::shape_type st) {
    switch (st) {
        case g4d::shape_type::st_circle : return "Circle";
        case g4d::shape_type::st_ellipse : return "Ellipse";
        case g4d::shape_type::st_line: return "Line";
        default : return "Line";
    }
}

namespace g4d {

playground::playground(const std::string_view& fonts_dir) : seltype{shape_type::st_line}, anchored{false}, f(font::load(fonts_dir, "kitchen_sink")), tex_r(f) {}

void playground::update(double dt){}

playground::shape playground::current_shape() const {
    constexpr int MAX_R = 4096;
    int rad = std::min(MAX_R, (int)std::lround(glm::distance(glm::vec2(mouse_pos), glm::vec2(anchor))));
    glm::ivec2 ax = glm::min(glm::abs(anchor - mouse_pos), glm::ivec2(MAX_R));
    ax.x = glm::max(ax.x, 2); 
    ax.y = glm::max(ax.y, 2); 

    switch (seltype) {
    case shape_type::st_circle:
        return circle{.pos{anchor}, .radius{rad}, .color{255, 255, 255, 255}};
    case shape_type::st_ellipse:
        return ellipse{.pos{anchor}, .paxis{ax}, .color{255, 255, 255, 255}};
    case shape_type::st_line:
        break;
    }
    return line{.pos1{anchor}, .pos2{mouse_pos}, .color{255, 255, 255, 255}};
}

void playground::draw(const playground::shape& shape, frame& f) {
    std::visit(overloaded{
        [&f](const circle& cir) { f.draw_circle(cir.pos.x, cir.pos.y, cir.radius, 0xffffffff); },
        [&f](const ellipse& ell) { f.draw_ellipse(ell.pos.x, ell.pos.y, ell.paxis.y, ell.paxis.x, 0xffffffff); },
        [&f](const line& lin) { f.draw_line(lin.pos1.x, lin.pos1.y, lin.pos2.x, lin.pos2.y, 0xffffffff); },
    }, shape);
}

void playground::render(frame& f) {

    tex_r.draw(f, 20, f.height() - 20, "Currently Drawing: " + shape_name(seltype) , {.scale{3}});
    for(auto& sh : shapes) draw(sh, f);
    if(anchored) draw(current_shape(), f);

}

void playground::on_key(int key, int action) {
    if(action != GLFW_PRESS) return;
    if (key >= GLFW_KEY_1 && key <= GLFW_KEY_3) {
        seltype = get_shape(key - GLFW_KEY_1);
        anchored = false;
    }
}

void playground::on_mouse_move(int x, int y) {
    mouse_pos = glm::ivec2(x, y);
}

void playground::on_mouse_button(int button, int action, int x, int y) {
    if(action == GLFW_PRESS && !anchored) {
        anchored = true;
        mouse_pos = glm::ivec2(x, y);
        anchor = glm::ivec2(x, y);
    } else if(action == GLFW_RELEASE && anchored) {
        shapes.push_back(current_shape());
        anchored = false;
    }
}

}