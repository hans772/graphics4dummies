#include "raster/frame.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main() {
    g4d::frame nf(100, 100);
    nf.draw_line_coh_sut(10, 10, 90, 90, 0xffffffff);
    nf.draw_line_coh_sut(40, 40, 200, 105, 0xffffffff);
    nf.draw_line_coh_sut(-20, 10, 30, 70, 0xffffffff);
    nf.draw_line_coh_sut(-20, 50, 50, -20, 0xffffffff);
    nf.draw_line_coh_sut(-20, 30, 120, 30, 0xffffffff);

    stbi_write_png("line_cs.png", 100, 100, 4, nf.get_buffer(), 400);
}