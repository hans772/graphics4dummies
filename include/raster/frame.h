#pragma once

#include <vector>
#include <cstdint>

namespace g4d {


class frame {
private:

    std::vector<uint32_t> frame_buffer;
    size_t w;
    size_t h;

public:

    frame(size_t width, size_t height);

    /**
     * @brief Draws a pixel on the frame buffer
     * 
     * @param x Pixel position on the horizontal axis
     * @param y Pixel position on the vertical axis
     * @param color Color of the pixel in 0xAARRGGBB format
     */
    
    void draw_pixel(int x, int y, uint32_t color);

    /**
     * @brief Draws 8 symmetric pixels about cx and cy
     * 
     * @param x Horizontal pixel position of NE pixel
     * @param y Vertical pixel position of NE pixel
     * @param cx Circle center on horizontal axis
     * @param cy Circle center on vertical axis
     * @param color Color of the 8 pixels
     */
    void draw_symmetric_pixel(int x, int y, int cx, int cy, uint32_t color, bool symm8 = false);

    /**
     * @brief Draws a line on the frame buffer
     * 
     * @warning Does not perform line clipping points outside the frame will be skipped (will still be iterated)
     * 
     * @param x1 Pixel position on the horizontal axis of the start point
     * @param y1 Pixel position on the vertical axis of the start point
     * @param x2 Pixel position on the horizontal axis of the end point
     * @param y2 Pixel position on the vertical axis of the end point
     * @param color Color of the line in 0xAARRGGBB format
     */

    void draw_line(int x1, int y1, int x2, int y2, uint32_t color);

    /**
     * @brief Draws a clipped line on the frame buffer using Cohen Sutherland line clipping algorithm
     * 
     * @param x1 Pixel position on the horizontal axis of the start point
     * @param y1 Pixel position on the vertical axis of the start point
     * @param x2 Pixel position on the horizontal axis of the end point
     * @param y2 Pixel position on the vertical axis of the end point
     * @param color Color of the line in 0xAARRGGBB format
     */
    void draw_line_coh_sut(int x1, int y1, int x2, int y2, uint32_t color);

    /**
     * @brief Draws a clipped line on the frame buffer using Cyrus Beck line clipping algorithm
     * 
     * @param x1 Pixel position on the horizontal axis of the start point
     * @param y1 Pixel position on the vertical axis of the start point
     * @param x2 Pixel position on the horizontal axis of the end point
     * @param y2 Pixel position on the vertical axis of the end point
     * @param color Color of the line in 0xAARRGGBB format
     */
    void draw_line_cyr_bec(int x1, int y1, int x2, int y2, uint32_t color);

    /**
     * @brief Draws a circle of radius r around the given center.
     * 
     * @warning Does not perform line clipping points outside the frame will be skipped (will still be iterated)
     * 
     * @param cx Pixel position on the horizontal axis of the center
     * @param cy Pixel position on the vertical axis of the center
     * @param r Radius of the circle
     * @param color Color of the circle in 0xAARRGGBB format
     */

    void draw_circle(int cx, int cy, size_t r, uint32_t color);

    /**
     * @brief Draws an ellipse of height: 2 * h and width: 2 * w around the given center.
     * 
     * @param cx Pixel position on the horizontal axis of the center
     * @param cy Pixel position on the vertical axis of the center
     * @param h Vertical half length of ellipse
     * @param w Horizontal half length of ellipse
     * @param color Color of the circle in 0xAARRGGBB format
     */
    void draw_ellipse(int cx, int cy, int h, int w, uint32_t color);

    /**
     * @brief Clears the frame buffer
     * 
     */

    void clear();

    size_t width() const {
        return w;
    }

    size_t height() const {
        return h;
    }

    const uint32_t* get_buffer() {
        return frame_buffer.data();
    }
};

}

