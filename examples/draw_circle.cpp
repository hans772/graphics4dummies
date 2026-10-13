#include "raster/frame.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main() {
    g4d::frame nf(500, 500);
    nf.draw_circle(250, 250, 245, 0xffffffff);
    stbi_write_png("circle.png", 500, 500, 4, nf.get_buffer(), 2000);
}