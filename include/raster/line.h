#pragma once

#include <cstdint>
#include "raster/frame.h"

namespace g4d {

/**
 * @brief Draws a line on the frame buffer
 * 
 * @warning Does not perform line clipping points outside the frame will be skipped (will still be iterated)
 * 
 * @param f Reference to the frame buffer
 * @param x1 Pixel position on the horizontal axis of the start point
 * @param y1 Pixel position on the vertical axis of the start point
 * @param x2 Pixel position on the horizontal axis of the end point
 * @param y2 Pixel position on the vertical axis of the end point
 * @param color Color of the line in 0xAARRGGBB format
 */
void draw_line(frame& f, int x1, int y1, int x2, int y2, color_t color);

/**
 * @brief Draws a clipped line on the frame buffer using Cohen Sutherland line clipping algorithm
 * 
 * @param f Reference to the frame buffer
 * @param x1 Pixel position on the horizontal axis of the start point
 * @param y1 Pixel position on the vertical axis of the start point
 * @param x2 Pixel position on the horizontal axis of the end point
 * @param y2 Pixel position on the vertical axis of the end point
 * @param color Color of the line in 0xAARRGGBB format
 */
void draw_line_coh_sut(frame& f, int x1, int y1, int x2, int y2, color_t color);

/**
 * @brief Draws a clipped line on the frame buffer using Cyrus Beck line clipping algorithm
 * 
 * @param f Reference to the frame buffer
 * @param x1 Pixel position on the horizontal axis of the start point
 * @param y1 Pixel position on the vertical axis of the start point
 * @param x2 Pixel position on the horizontal axis of the end point
 * @param y2 Pixel position on the vertical axis of the end point
 * @param color Color of the line in 0xAARRGGBB format
 */
void draw_line_cyr_bec(frame& f, int x1, int y1, int x2, int y2, color_t color);

}  // namespace g4d
