#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

#include "util/font.h"

namespace g4d {

class frame;

/** @brief Look of a piece of text. All spacing values are in unscaled font pixels. */
struct text_style {
    uint32_t color = 0xffffffff;               ///< glyph colour, 0xAARRGGBB
    int scale = 1;                             ///< integer pixel multiplier (each font pixel becomes scale x scale)
    int letter_spacing = 0;                    ///< extra pixels between glyph cells
    int line_spacing = 0;                      ///< extra pixels between lines
    std::optional<uint32_t> background;        ///< if set, unlit pixels of every glyph cell are filled with this
};

/** @brief Size in pixels of a rendered block of text. */
struct text_extent {
    int width = 0;
    int height = 0;
};

/**
 * @brief Rasterizes strings into a g4d::frame using a g4d::font.
 *
 * Coordinates use the frame's convention (origin bottom-left, y up). The anchor (x, y) of
 * draw() is the TOP-LEFT pixel of the text block; the text extends to the right and downward
 * (towards smaller y). '\n' starts a new line, '\r' is ignored.
 *
 * Strings are read byte by byte, so each byte is one character code (0-255). That matches
 * ASCII exactly; UTF-8 multi-byte characters appear as several glyphs. Characters without a
 * glyph in the font are skipped but still advance the pen.
 *
 * Everything goes through frame::draw_pixel, so text partly or fully outside the frame is
 * clipped safely.
 *
 * The renderer keeps a reference to the font, so the font must outlive it.
 */
class text_raster {
public:
    explicit text_raster(const font& f) : font_(f) {}
    text_raster(font&&) = delete; 

    /** @brief Draws `text` with its top-left corner at (x, y). */
    void draw(frame& f, int x, int y, std::string_view text, const text_style& style = {}) const;

    /** @brief Draws `text` so the middle of its block is at (cx, cy). Good for node labels. */
    void draw_centered(frame& f, int cx, int cy, std::string_view text, const text_style& style = {}) const;

    /**
     * @brief Size of the block draw() would produce. Letter spacing is not added after the
     *        last glyph of a line, nor line spacing after the last line. Empty text is 0x0.
     */
    text_extent measure(std::string_view text, const text_style& style = {}) const;

    const font& get_font() const { return font_; }

private:
    const font& font_;
};

}  // namespace g4d