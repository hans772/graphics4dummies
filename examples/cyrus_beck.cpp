#include "raster/frame.h"
#include "raster/line.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main() {
    g4d::frame nf(100, 100);
    g4d::draw_line_cyr_bec(nf, 10, 10, 90, 90, g4d::color_t::from_hex(0xffffffff));
    g4d::draw_line_cyr_bec(nf, 40, 40, 200, 105, g4d::color_t::from_hex(0xffffffff));
    g4d::draw_line_cyr_bec(nf, -20, 10, 30, 70, g4d::color_t::from_hex(0xffffffff));
    g4d::draw_line_cyr_bec(nf, -20, 50, 50, -20, g4d::color_t::from_hex(0xffffffff));
    g4d::draw_line_cyr_bec(nf, -20, 30, 120, 30, g4d::color_t::from_hex(0xffffffff));

    stbi_write_png("line_cb.png", 100, 100, 4, nf.get_buffer(), 400);
}
