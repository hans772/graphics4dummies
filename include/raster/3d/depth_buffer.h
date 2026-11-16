#pragma once

#include <vector>
#include "util/color.h"

namespace g4d {

class depth_buffer {
    
    size_t _width;
    size_t _height;
    std::vector<float> _dbuf;

public:

    depth_buffer(size_t width, size_t height);
    bool test_and_set(int x, int y, float invw);

    size_t width() const;
    size_t height() const;

    void resize(size_t width, size_t height);

    void clear(float clear_value = 0.0f);

};

}