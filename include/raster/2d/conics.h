#pragma once

#include <cstddef>
#include <cstdint>
#include "raster/frame.h"

namespace g4d {

/**
 * @brief Draws 8 symmetric pixels about cx and cy
 * 
 * @param f Reference to the frame buffer
 * @param x Horizontal pixel position of NE pixel
 * @param y Vertical pixel position of NE pixel
 * @param cx Circle center on horizontal axis
 * @param cy Circle center on vertical axis
 * @param color Color of the 8 pixels
 * @param symm8 Whether to draw 8-way symmetry (true for circle, false for 4-way ellipse)
 */
void draw_symmetric_pixel(frame& f, int x, int y, int cx, int cy, color_t color, bool symm8 = false);

/**
 * @brief Draws a circle of radius r around the given center.
 * 
 * @warning Does not perform line clipping points outside the frame will be skipped (will still be iterated)
 * 
 * @param f Reference to the frame buffer
 * @param cx Pixel position on the horizontal axis of the center
 * @param cy Pixel position on the vertical axis of the center
 * @param r Radius of the circle
 * @param color Color of the circle in 0xAARRGGBB format
 */
void draw_circle(frame& f, int cx, int cy, size_t r, color_t color);

/**
 * @brief Draws an ellipse of height: 2 * h and width: 2 * w around the given center.
 * 
 * @param f Reference to the frame buffer
 * @param cx Pixel position on the horizontal axis of the center
 * @param cy Pixel position on the vertical axis of the center
 * @param h Vertical half length of ellipse
 * @param w Horizontal half length of ellipse
 * @param color Color of the circle in 0xAARRGGBB format
 */
void draw_ellipse(frame& f, int cx, int cy, int h, int w, color_t color);

}  // namespace g4d
