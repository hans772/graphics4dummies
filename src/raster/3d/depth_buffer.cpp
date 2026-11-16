#include "raster/3d/depth_buffer.h"

namespace g4d
{
    
void depth_buffer::clear(float clear_value) {
        std::fill(_dbuf.begin(), _dbuf.end(), clear_value);
    }

void depth_buffer::resize(size_t width, size_t height) {
        _width = width;
        _height = height;
        _dbuf.assign(width * height, 0.0f);
    }

size_t depth_buffer::width() const { return _width; }
size_t depth_buffer::height() const { return _height; }

depth_buffer::depth_buffer(size_t width, size_t height) : _width(width), _height(height), _dbuf(width*height, 0)  {}

bool depth_buffer::test_and_set(int x, int y, float invw) {
    if (x < 0 || x >= static_cast<int>(_width) || y < 0 || y >= static_cast<int>(_height))
        return false;
    
    size_t idx = static_cast<size_t>(y) * _width + static_cast<size_t>(x);
    if (invw > _dbuf[idx]) {
        _dbuf[idx] = invw;
        return true;
    }
    return false;
}

} // namespace g4d
