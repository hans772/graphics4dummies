#include "raster/frame.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main() {
    g4d::frame nf(100, 100);
    nf.draw_ellipse(50, 50, 30, 20, 0xffffffff);
    stbi_write_png("ellipse.png", 100, 100, 4, nf.get_buffer(), 400);
}