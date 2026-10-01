#include "raster/frame.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main() {
    g4d::frame nf(100, 100);
    nf.draw_line(10, 10, 40, 80, 0xffffffff);
    stbi_write_png("line.png", 100, 100, 4, nf.get_buffer(), 400);
}