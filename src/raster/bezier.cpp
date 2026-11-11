#include "raster/bezier.h"

namespace g4d
{

    bezier::bezier(glm::vec2 p1, glm::vec2 p2): _n_pts(2), _control_pts{p1, p2}, _poly_line{p1, p2}, _tolerance(0.0f) {}

    glm::vec2 bezier::sample(double t) {
        std::vector<glm::dvec2> pts(_control_pts.begin(), _control_pts.end());
        size_t n = _n_pts;

        while(n > 1) {
            for(int i = 0; i < n-1; i++) {
                pts[i] = pts[i] * (1-t) + pts[i+1] * t;
            }
            n--;
        }
        
        return pts[0];
        
    }
    
    void bezier::reapproximate() {
        if(_n_pts < 3) {
            _poly_line = {*_control_pts.begin(), *_control_pts.rbegin()};
            return;
        }

        auto pi = _control_pts.begin();
        auto pi1 = std::next(pi);
        auto pi2 = std::next(pi1);

        double M = 0;
        while(pi2 != _control_pts.end()) {
            M = std::max(M, (double) glm::length(*pi2 - (2.0f * (*pi1)) + *pi));
            std::advance(pi, 1);
            std::advance(pi1, 1);
            std::advance(pi2, 1);
        }

        size_t num_subdivs = std::ceil(std::sqrt(_n_pts * (_n_pts-1) * M / (8 * _tolerance)));
        num_subdivs = std::max(num_subdivs, (size_t)1);

        _poly_line.resize(num_subdivs+1);

        for(int k = 0; k <= num_subdivs; k++) {
            _poly_line[k] = sample(((double)k)/num_subdivs);
        }
    }

    void bezier::add_point(glm::vec2 p, bool reapprox){
        size_t i{get_closest_point(p)};
        _control_pts.insert(std::next(_control_pts.begin(), i), p);
        _n_pts++;
        if(reapprox) reapproximate();
    }

    void bezier::delete_point(size_t i, bool reapprox) {
        if(_n_pts < 3) return;
        _control_pts.erase(std::next(_control_pts.begin(), i));
        _n_pts--;
        if(reapprox) reapproximate();
    }

    void bezier::reposition_point(size_t i, glm::vec2 p, bool reapprox) {
        (*std::next(_control_pts.begin(), i)) = p;
        if(reapprox) reapproximate();
    }
    
    void bezier::set_tolerance(double t, bool reapprox) {
        _tolerance = t;
        if(reapprox) reapproximate();
    }

    size_t bezier::get_closest_point(glm::vec2 p) {
        int i = 0;
        double dist{glm::length(glm::vec2(p) - *_control_pts.begin())};
        auto pt{_control_pts.begin()};
        for(int idx{0}; idx < _control_pts.size(); idx++, std::advance(pt, 1)) {
            double nd{glm::length(glm::vec2(p) - *pt)};
            if(nd < dist) {
                i = idx;
                dist = nd;
            }
        }

        return i;
    }

    void bezier::draw(g4d::frame& f, int x, int y, size_t subdivisions, g4d::color_t color) {
        for(int t = 0; t <= subdivisions; t++) {
            auto pt = sample(((double)t)/subdivisions);
            f.draw_pixel(x + (int)pt.x, y + (int)pt.y, color);
        }
    }

    void bezier::draw_approx(g4d::frame& f, int x, int y, g4d::color_t color){
        for(int i = 0; i < _poly_line.size()-1; i++) {
            auto& pc = _poly_line[i];
            auto& pn = _poly_line[i+1];
            draw_line_cyr_bec(f, x+pc.x, y+pc.y, x+pn.x, y+pn.y, color);
        }
    }

    const std::list<glm::vec2>& bezier::control_points() const {
        return _control_pts;
    }

} // namespace g4d
