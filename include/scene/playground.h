#pragma once

#include "scene/scene.h"
#include "util/shapes.h"
#include <variant>

namespace g4d {

class playground : public scene {
public:
    playground();
    const char* name() const override { return "playground"; }
    void update(double dt) override;
    void render(frame& f) override;
    void on_key(int key, int action) override;
    void on_mouse_move(int x, int y) override;
    void on_mouse_button(int button, int action, int x, int y) override;

private:


    using shape = std::variant<circle, ellipse, line>;
    std::vector<shape> shapes;

    shape_type seltype;
    bool anchored{false};
    glm::ivec2 anchor{0};
    glm::ivec2 mouse_pos{0};


    shape current_shape() const;
    void draw(const shape&, frame& f);
};

}