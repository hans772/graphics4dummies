#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace g4d {

/**
 * @brief A fixed-cell bitmap font loaded from a glyph sheet (one PNG holding a grid of equally sized glyph cells) and a manifest.
 *
 * Folder layout:
 * @code
 * fonts/
 *   kitchen-sink/
 *     manifest.json      (or a file named just "manifest")
 *     kitchen-sink.png
 * @endcode
 *
 * Manifest (Custom file with 4 rows)
 * @code
 * [filename]          "<glyph sheet filename relative to this directory wrapped in quotes>"
 * [channels]          <number of channels of the image> 
 * [start_offset]      <ascii value of the first ascii character in the glyph sheet>
 * [glyphs_per_row]    <number of glyphs per row>
 * [glyphs_per_col]    <number of glyphs per columns>
 * @endcode
 *
 * A glyph pixel is "lit" if it is opaque and light (luminance > 127) when the image has
 * no transparency, or if its alpha is > 127 when the image has transparent pixels.
 * (So white-on-black and anything-on-transparent sheets work; black-on-white would be inverted.)
 */
class font {
public:
    /**
     * @brief Loads `<fonts_dir>/<name>/`.
     * @throws std::runtime_error with a message naming the file and the problem
     */
    static font load(const std::filesystem::path& fonts_dir, const std::string& name);

    /** @brief Loads a font from one specific font folder (the one containing the manifest). */
    static font load_from_directory(const std::filesystem::path& font_dir);

    const std::string& name() const { return font_name; }
    int glyph_width() const { return cell_w; }
    int glyph_height() const { return cell_h; }
    int first_code() const { return first_off; }
    int glyph_count() const { return per_row * per_col; }

    /** @brief True if the sheet has a cell for this character code. */
    bool has_glyph(int code) const {
        int i = code - first_off;
        return i >= 0 && i < glyph_count();
    }

    /**
     * @brief Is pixel (x, y) of the glyph for `code` lit? y = 0 is the TOP row of the glyph.
     * @return false for codes without a glyph and for coordinates outside the cell
     */
    bool lit(int code, int x, int y) const {
        const int i = code - first_off;
        if (i < 0 || i >= glyph_count() || x < 0 || x >= cell_w || y < 0 || y >= cell_h) return false;
        const int row = i / per_row, col = i % per_row;
        return atlas[(size_t)(row * cell_h + y) * atlas_w + col * cell_w + x] != 0;
    }

private:
    font() = default;

    std::string font_name;
    int cell_w = 0, cell_h = 0;
    int per_row = 0, per_col = 0;
    int first_off = 0;
    int atlas_w = 0;
    std::vector<uint8_t> atlas;   // whole sheet, 1 = lit
};

}  // namespace g4d