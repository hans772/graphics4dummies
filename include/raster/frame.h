#pragma once

#include <vector>
#include <cstdint>
#include <cstddef>
#include "util/color.h"
namespace g4d {

/**
 * @brief A frame class that stores every pixel on screen as a 32 bit unsigned int buffer, which is converted into a texture and rendered every frame
 * 
 * @warning There are some issues with coloring wrt little endian and big endian architectures
 * 
 */
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
    void draw_pixel(int x, int y, color_t color);

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

    const uint32_t* get_buffer() const {
        return frame_buffer.data();
    }

    uint32_t* get_buffer() {
        return frame_buffer.data();
    }
};

}  // namespace g4d
