#include "raster/frame.h"
#include <algorithm>

namespace g4d
{

frame::frame(size_t width, size_t height) : 
    frame_buffer(width * height, color_t::rgba(0, 0, 0, 255)), 
    w(width), 
    h(height) 
{}

void frame::draw_pixel(int x, int y, color_t color)
{
    if (x < 0 || x >= w || y < 0 || y >= h)
        return;
    frame_buffer[(h - y - 1) * w + x] = color.get();
}

void frame::clear() { std::fill(frame_buffer.begin(), frame_buffer.end(), color_t::rgba(0, 0, 0, 255)); }

}  // namespace g4d
