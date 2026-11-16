#pragma once

#include "raster/3d/depth_buffer.h"
#include "raster/3d/camera.h"
#include "object/object.h"
#include "raster/frame.h"

namespace g4d {

class rasterizer3d {

    depth_buffer _d_buf;
    
public:

    camera cam;

    rasterizer3d(int width, int height);

    size_t width() const;
    size_t height() const;

    void resize(size_t width, size_t height);
    void clear();
    void raster(frame& f, const object& obj);
};

}