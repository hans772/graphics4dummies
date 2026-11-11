#pragma once

#include "raster/frame.h"
#include <vector>
#include <list>
#include <glm/glm.hpp>
#include "raster/line.h"

namespace g4d {

class bezier {
    std::list<glm::vec2> _control_pts;
    size_t _n_pts;
    std::vector<glm::ivec2> _poly_line;
    double _tolerance;

public:
    bezier(glm::vec2 p1, glm::vec2 p2);

    glm::vec2 sample(double t);
    void reapproximate();
    void add_point(glm::vec2 p, bool reapprox = true);
    void delete_point(size_t i, bool reapprox = true);
    void reposition_point(size_t i, glm::vec2 p, bool reapprox = true);
    void set_tolerance(double t, bool reapprox = true);
    size_t get_closest_point(glm::vec2 p);

    void draw(g4d::frame& f, int x, int y, size_t subdivisions, g4d::color_t color);
    void draw_approx(g4d::frame& f, int x, int y, g4d::color_t color);

    const std::list<glm::vec2>& control_points() const;

};

}