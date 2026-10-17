#include "raster/text.h"

#include <algorithm>

#include "raster/frame.h"

namespace g4d {

text_extent text_raster::measure(std::string_view text, const text_style& s) const {
    if (text.empty()) return {};
    const int sc = std::max(1, s.scale);

    int lines = 1, longest = 0, current = 0;
    for (unsigned char c : text) {
        if (c == '\n') { lines++; current = 0; }
        else if (c != '\r') longest = std::max(longest, ++current);
    }

    text_extent e;
    if (longest > 0) e.width = (longest * (font_.glyph_width() + s.letter_spacing) - s.letter_spacing) * sc;
    e.height = (lines * (font_.glyph_height() + s.line_spacing) - s.line_spacing) * sc;
    return e;
}

void text_raster::draw(frame& f, int x, int y, std::string_view text, const text_style& s) const {
    const int sc = std::max(1, s.scale);
    const int cw = font_.glyph_width(), ch = font_.glyph_height();

    int pen_x = x, pen_y = y;   // top-left pixel of the current glyph cell
    for (unsigned char code : text) {
        if (code == '\n') {
            pen_x = x;
            pen_y -= (ch + s.line_spacing) * sc;
            continue;
        }
        if (code == '\r') continue;

        for (int gy = 0; gy < ch; gy++) {
            for (int gx = 0; gx < cw; gx++) {
                const bool lit = font_.lit(code, gx, gy);
                if (!lit && !s.background) continue;
                const uint32_t color = lit ? s.color : *s.background;

                const int px = pen_x + gx * sc;
                const int py = pen_y - gy * sc;                 // glyph row 0 is the top; y points up
                for (int dy = 0; dy < sc; dy++)
                    for (int dx = 0; dx < sc; dx++) f.draw_pixel(px + dx, py - dy, color);
            }
        }
        pen_x += (cw + s.letter_spacing) * sc;
    }
}

void text_raster::draw_centered(frame& f, int cx, int cy, std::string_view text, const text_style& s) const {
    const text_extent e = measure(text, s);
    if (e.width == 0 && e.height == 0) return;
    // top-left of a block whose middle lands on (cx, cy); the block covers rows y .. y-height+1
    draw(f, cx - e.width / 2, cy + e.height / 2 - 1 + (e.height % 2), text, s);
}

}  // namespace g4d