#include "raster/2d/conics.h"

namespace g4d {

void draw_symmetric_pixel(frame& f, int x, int y, int cx, int cy, color_t color, bool symm8)
{
    f.draw_pixel(x, y, color);
    f.draw_pixel(2 * cx - x, y, color);
    f.draw_pixel(2 * cx - x, 2 * cy - y, color);
    f.draw_pixel(x, 2 * cy - y, color);

    if(!symm8) return;

    f.draw_pixel(cx + (y - cy), cy + (x - cx), color);
    f.draw_pixel(cx + cy - y, cy + (x - cx), color);
    f.draw_pixel(cx + cy - y, cy + cx - x, color);
    f.draw_pixel(cx + (y - cy), cy + cx - x, color);
}

void draw_circle(frame& f, int cx, int cy, size_t r, color_t color)
{
    int px = cx;
    int py = cy + r;
    int h = 1 - r;
    int incE = 3;
    int incSE = 5 - 2 * r;
    draw_symmetric_pixel(f, px, py, cx, cy, color, true);
    while (py - cy > px - cx)
    {
        if (h < 0)
        {
            h += incE;
            incE += 2;
            incSE += 2;
            px++;
        }
        else
        {
            h += incSE;
            incE += 2;
            incSE += 4;
            px++;
            py--;
        }
        draw_symmetric_pixel(f, px, py, cx, cy, color, true);
    }
}

void draw_ellipse(frame& f, int cx, int cy, int h, int w, color_t color) {
    long long a = w; 
    long long b = h; 
    long long a2 = a * a;
    long long b2 = b * b;

    long long px = 0;
    long long py = b;

    long long d1 = 4 * b2 - 4 * a2 * b + a2;
    long long dx = 2 * b2 * px;
    long long dy = 2 * a2 * py;

    while (dx < dy) {
        draw_symmetric_pixel(f, cx + px, cy + py, cx, cy, color);

        px++;
        dx += 2 * b2;
        if (d1 < 0) {
            d1 += 4 * b2 * (2 * px + 1);
        } else {
            py--;
            dy -= 2 * a2;
            d1 += 4 * b2 * (2 * px + 1) - 4 * a2 * (2 * py);
        }
    }

    long long d2 = b2 * (2 * px + 1) * (2 * px + 1) 
                 + 4 * a2 * (py - 1) * (py - 1) 
                 - 4 * a2 * b2;

    while (py >= 0) {
        draw_symmetric_pixel(f, cx + px, cy + py, cx, cy, color);

        py--;
        dy -= 2 * a2;
        if (d2 > 0) {
            d2 += 4 * a2 * (3 - 2 * py);
        } else {
            px++;
            dx += 2 * b2;
            d2 += 4 * b2 * (2 * px + 2) + 4 * a2 * (3 - 2 * py);
        }
    }
}

}  // namespace g4d
