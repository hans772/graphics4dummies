
#include "raster/frame.h"
#include <utility>
#include <math.h>
#include <glm/glm.hpp>
#include <bit>

namespace g4d
{

frame::frame(size_t width, size_t height) : frame_buffer(width * height, 0xff000000), w(width), h(height) {}

void frame::draw_pixel(int x, int y, uint32_t color)
{
    if (x < 0 || x >= w || y < 0 || y >= h)
        return;
    frame_buffer[(h - y - 1) * w + x] = color;
}

void frame::draw_symmetric_pixel(int x, int y, int cx, int cy, uint32_t color, bool symm8)
{
    draw_pixel(x, y, color);
    draw_pixel(2 * cx - x, y, color);
    draw_pixel(2 * cx - x, 2 * cy - y, color);
    draw_pixel(x, 2 * cy - y, color);

    if(!symm8) return;

    draw_pixel(cx + (y - cy), cy + (x - cx), color);
    draw_pixel(cx + cy - y, cy + (x - cx), color);
    draw_pixel(cx + cy - y, cy + cx - x, color);
    draw_pixel(cx + (y - cy), cy + cx - x, color);
}

void frame::draw_line(int x1, int y1, int x2, int y2, uint32_t color)
{
    int dx = std::abs(x2 - x1), dy = std::abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1, sy = y1 < y2 ? 1 : -1;
    bool steep = dy > dx;
    if (steep)
        std::swap(dx, dy);
    int d = 2 * dy - dx;
    int x = x1, y = y1;
    for (int i = 0; i <= dx; i++)
    {
        draw_pixel(x, y, color);
        if (d > 0)
        {
            if (steep)
                x += sx;
            else
                y += sy;
            d -= 2 * dx;
        }
        d += 2 * dy;
        if (steep)
            y += sy;
        else
            x += sx;
    }

}

void frame::draw_line_coh_sut(int x1, int y1, int x2, int y2, uint32_t color)
{
    constexpr uint8_t INSIDE = 0b0000;
    constexpr uint8_t LEFT   = 0b0001;
    constexpr uint8_t RIGHT  = 0b0010;
    constexpr uint8_t BOTTOM = 0b0100;
    constexpr uint8_t TOP    = 0b1000;
    
    const double x_min = 0.0;
    const double x_max = static_cast<double>(w - 1);
    const double y_min = 0.0;
    const double y_max = static_cast<double>(h - 1);

    auto compute_outcode = [&](double x, double y) -> uint8_t {
        uint8_t code = INSIDE;
        if (x < x_min)      code |= LEFT;
        else if (x > x_max) code |= RIGHT;
        if (y < y_min)      code |= BOTTOM;
        else if (y > y_max) code |= TOP;
        return code;
    };

    double x_a = x1, y_a = y1;
    double x_b = x2, y_b = y2;

    uint8_t code_a = compute_outcode(x_a, y_a);
    uint8_t code_b = compute_outcode(x_b, y_b);

    while (true) {
        if ((code_a | code_b) == 0) {
            draw_line(
                static_cast<int>(std::round(x_a)),
                static_cast<int>(std::round(y_a)),
                static_cast<int>(std::round(x_b)),
                static_cast<int>(std::round(y_b)),
                color
            );
            return;
        }

        if(code_a & code_b) return;

        uint8_t out_c = code_a ? code_a : code_b;
        double x = 0.0, y = 0.0;

        if (out_c & TOP) {
            x = x_a + (x_b - x_a) * (y_max - y_a) / (y_b - y_a);
            y = y_max;
        } else if (out_c & BOTTOM) {
            x = x_a + (x_b - x_a) * (y_min - y_a) / (y_b - y_a);
            y = y_min;
        } else if (out_c & RIGHT) {
            y = y_a + (y_b - y_a) * (x_max - x_a) / (x_b - x_a);
            x = x_max;
        } else if (out_c & LEFT) {
            y = y_a + (y_b - y_a) * (x_min - x_a) / (x_b - x_a);
            x = x_min;
        }

        if (out_c == code_a) {
            x_a = x;
            y_a = y;
            code_a = compute_outcode(x_a, y_a);
        } else {
            x_b = x;
            y_b = y;
            code_b = compute_outcode(x_b, y_b);
        }
        
    }
}

void frame::draw_line_cyr_bec(int x1, int y1, int x2, int y2, uint32_t color) {
    float tl = 1.0f;
    float te = 0.0f;

    const glm::vec2 normals[4]{{1, 0},{0, 1},{-1, 0},{0, -1}};
    const glm::vec2 refs[2]{{0, 0}, {w-1, h-1}};
    
    glm::vec2 p0{x1, y1};
    glm::vec2 p1{x2, y2};
    glm::vec2 D{p1-p0};
    for(int i = 0; i < 4; i++) {
        glm::vec2 w(p0 - refs[i>>1]);
        float wn = glm::dot(normals[i], w);
        float dn = glm::dot(D, normals[i]);

        if(dn==0.0f) {
            if(wn < 0) return;
            else continue;
        }

        float t = - wn / dn;
        
        if(dn > 0) { // PE
            if(t > 1.0) return;
            te = std::max(t, te);
        } else {
            if(t < 0.0) return;
            tl = std::min(tl, t);
        }
    }

    if(tl <= te) return;
    glm::vec2 s = p0 + te * D;
    glm::vec2 e = p0 + tl * D;

    draw_line(
        static_cast<int>(std::round(s.x)),
        static_cast<int>(std::round(s.y)),
        static_cast<int>(std::round(e.x)),
        static_cast<int>(std::round(e.y)),
        color
    );
}

void frame::draw_circle(int cx, int cy, size_t r, uint32_t color)
{
    int px = cx;
    int py = cy + r;
    int h = 1 - r;
    int incE = 3;
    int incSE = 5 - 2 * r;
    draw_symmetric_pixel(px, py, cx, cy, color, true);
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
        draw_symmetric_pixel(px, py, cx, cy, color, true);
    }
}

void frame::draw_ellipse(int cx, int cy, int h, int w, uint32_t color) {
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
        draw_symmetric_pixel(cx + px, cy + py, cx, cy, color);

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
        draw_symmetric_pixel(cx + px, cy + py, cx, cy, color);

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

void frame::clear() { std::fill(frame_buffer.begin(), frame_buffer.end(), 0xff000000); }
}