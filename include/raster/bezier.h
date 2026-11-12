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
    /**
     * @brief Construct a new bezier object with 2 points
     * 
     * @param p1 first point   
     * @param p2 second point
     */
    bezier(glm::vec2 p1, glm::vec2 p2);

    /**
     * @brief returns the value of the parametric bezier curve given parameter t
     * 
     * @param t parameter for the bezier curve [0, 1]
     * @return glm::vec2 point on the curve that corresponds to the parameter
     */
    glm::vec2 sample(double t) const;

    /**
     * @brief reapproximates the polyline approximation with respect to the tolerance using wang's flatness
     * 
     */
    void reapproximate();

    /**
     * @brief adds a point to the curve, point will be added before the closest point to it among the control points
     * 
     * @param p the point to be added
     * @param reapprox whether to reapproximate the polyline
     */
    void add_point(glm::vec2 p, bool reapprox = true);

    /**
     * @brief deletes a point wrt it's index among the control points
     * 
     * @param i index of the point to be removed
     * @param reapprox whether to reapproximate the polyline
     */
    void delete_point(size_t i, bool reapprox = true);

    /**
     * @brief repositions the chosen point to the given coordinate
     * 
     * @param i index of the point to be adjusted
     * @param p position the point is to be adjusted to
     * @param reapprox whether to reapproximate the polyline
     */
    void reposition_point(size_t i, glm::vec2 p, bool reapprox = true);

    /**
     * @brief Set the tolerance object
     * 
     * @param t new tolerance
     * @param reapprox whether to reapproximate the polyline
     */
    void set_tolerance(double t, bool reapprox = true);
    
    /**
     * @brief return the closest point to the given point
     * 
     * @param p the point to be compared to
     * @return size_t the index of the closest point 
     */
    size_t get_closest_point(glm::vec2 p);

    /**
     * @brief samples and draws the bezier curve `subdivisions` number of times
     * 
     * @param f the frame to be drawn on
     * @param x the point at which the bezier curve starts [x]
     * @param y the point at which the bezier curve starts [y]
     * @param subdivisions number of subdivisions to draw
     * @param color color of the bezier curve
     * 
     * @warning this method of drawing only samples `subdivisions` number of times 
     * and draws those specific pixels, too little subdivisions will result in a disconnected curve 
     */
    void draw(g4d::frame& f, int x, int y, size_t subdivisions, g4d::color_t color);

    /**
     * @brief draws the polyline approximation of the bezier curve on the frame
     * 
     * @param f the frame to be drawn onto
     * @param x the point at which the bezier curve starts [x]
     * @param y the point at which the bezier curve starts [y]
     * @param color the color of the curve
     */
    void draw_approx(g4d::frame& f, int x, int y, g4d::color_t color);

    /**
     * @brief get control points
     * 
     * @return const std::list<glm::vec2>& control points 
     */
    const std::list<glm::vec2>& control_points() const;

    /**
     * @brief Get the polyline approximation object
     * 
     * @return const std::vector<glm::ivec2>& the polyline approximation 
     */
    const std::vector<glm::ivec2>& get_line() const;

    /**
     * @brief sample the curve 1/step times and returns the points
     * 
     * @param step the amount to step by each time [0, 1]
     * @return std::vector<glm::vec2> the points on the line
     */
    
    std::vector<glm::vec2> sample_line(double step) const;

};

}