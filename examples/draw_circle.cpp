#include "raster/frame.h"
#include "raster/conics.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main() {
    g4d::frame nf(500, 500);
    g4d::draw_circle(nf, 250, 250, 245, g4d::color_t::from_hex(0xffffffff));
    stbi_write_png("circle.png", 500, 500, 4, nf.get_buffer(), 2000);
}
