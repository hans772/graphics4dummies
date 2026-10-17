#include "util/font.h"

#include <cctype>
#include <fstream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb/stb_image.h"

namespace g4d {

namespace {

namespace fs = std::filesystem;

struct raw_value {
    std::string text;      // for quoted values: the contents without the quotes
    bool quoted = false;
    int line = 0;
};

std::string trim(const std::string& s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace((unsigned char)s[a])) a++;
    while (b > a && std::isspace((unsigned char)s[b - 1])) b--;
    return s.substr(a, b - a);
}

// Reads lines of the form   [key]   value
// - value is either "quoted text" (\" and \\ are the only escapes) or a bare token
// - blank lines and lines starting with # or ; are comments
// - CRLF line endings and a UTF-8 byte order mark are fine
std::map<std::string, raw_value> parse_manifest_text(const std::string& text, const std::string& where) {
    std::map<std::string, raw_value> out;
    std::istringstream in(text);
    std::string line;
    int line_no = 0;

    while (std::getline(in, line)) {
        line_no++;
        auto fail = [&](const std::string& msg) {
            throw std::runtime_error(where + ":" + std::to_string(line_no) + ": " + msg);
        };
        if (line_no == 1 && line.compare(0, 3, "\xEF\xBB\xBF") == 0) line.erase(0, 3);

        const std::string t = trim(line);
        if (t.empty() || t[0] == '#' || t[0] == ';') continue;
        if (t[0] != '[') fail("expected a line of the form [key] value");

        const size_t close = t.find(']');
        if (close == std::string::npos) fail("missing ']' after the key");
        const std::string key = trim(t.substr(1, close - 1));
        if (key.empty()) fail("empty key in []");
        const std::string rest = trim(t.substr(close + 1));
        if (rest.empty()) fail("missing value for [" + key + "]");
        if (out.count(key)) fail("duplicate key [" + key + "]");

        raw_value v;
        v.line = line_no;
        if (rest[0] == '"') {
            v.quoted = true;
            size_t i = 1;
            bool closed = false;
            while (i < rest.size()) {
                char c = rest[i++];
                if (c == '"') { closed = true; break; }
                if (c == '\\' && i < rest.size() && (rest[i] == '"' || rest[i] == '\\')) c = rest[i++];
                v.text += c;
            }
            if (!closed) fail("unterminated quoted string");
            if (!trim(rest.substr(i)).empty()) fail("unexpected text after the closing quote");
        } else {
            v.text = rest;
        }
        out[key] = v;
    }
    return out;
}

struct manifest {
    std::string filename;
    int start_offset = 0;
    int glyphs_per_row = 0;
    int glyphs_per_col = 0;
};

manifest read_manifest(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) throw std::runtime_error("cannot open " + path.string());
    std::ostringstream ss;
    ss << in.rdbuf();
    const std::string where = path.string();
    const auto values = parse_manifest_text(ss.str(), where);

    auto missing = [&](const char* key) { return std::runtime_error(where + ": missing key [" + key + "]"); };

    auto get_string = [&](const char* key) {
        auto it = values.find(key);
        if (it == values.end()) throw missing(key);
        return it->second.text;
    };
    auto get_int = [&](const char* key, bool required, int fallback) {
        auto it = values.find(key);
        if (it == values.end()) {
            if (required) throw missing(key);
            return fallback;
        }
        const raw_value& v = it->second;
        auto bad = [&](const char* why) {
            return std::runtime_error(where + ":" + std::to_string(v.line) + ": [" + key + "] " + why);
        };
        if (v.quoted) throw bad("must be an integer, not a quoted string");
        long long n = 0;
        size_t used = 0;
        try { n = std::stoll(v.text, &used); } catch (...) { throw bad("must be an integer"); }
        if (used != v.text.size()) throw bad("must be an integer");
        if (n < 0 || n > 100000) throw bad("is out of range (0 to 100000)");
        return (int)n;
    };

    manifest m;
    m.filename = get_string("filename");
    m.start_offset = get_int("start_offset", false, 0);
    m.glyphs_per_row = get_int("glyphs_per_row", true, 0);
    m.glyphs_per_col = get_int("glyphs_per_col", true, 0);
    if (m.filename.empty()) throw std::runtime_error(where + ": [filename] is empty");
    if (m.glyphs_per_row < 1 || m.glyphs_per_col < 1)
        throw std::runtime_error(where + ": glyphs_per_row and glyphs_per_col must be >= 1");
    return m;
}

}  // namespace

font font::load(const fs::path& fonts_dir, const std::string& name) {
    return load_from_directory(fonts_dir / name);
}

font font::load_from_directory(const fs::path& dir) {
    if (!fs::is_directory(dir)) throw std::runtime_error("font folder not found: " + dir.string());

    fs::path manifest_path;
    for (const char* candidate : {"manifest", "manifest.txt", "manifest.json"}) {
        if (fs::is_regular_file(dir / candidate)) { manifest_path = dir / candidate; break; }
    }
    if (manifest_path.empty())
        throw std::runtime_error("no manifest file (manifest, manifest.txt or manifest.json) in " + dir.string());

    const manifest m = read_manifest(manifest_path);

    const fs::path image_path = dir / m.filename;
    if (!fs::exists(image_path)) throw std::runtime_error("image named in the manifest not found: " + image_path.string());

    int w = 0, h = 0, channels = 0;
    std::unique_ptr<stbi_uc, decltype(&stbi_image_free)> pixels(
        stbi_load(image_path.string().c_str(), &w, &h, &channels, 4), &stbi_image_free);
    if (!pixels) throw std::runtime_error("cannot decode " + image_path.string() + ": " + stbi_failure_reason());

    if (w % m.glyphs_per_row != 0 || h % m.glyphs_per_col != 0)
        throw std::runtime_error(image_path.string() + ": image is " + std::to_string(w) + "x" + std::to_string(h) +
                                 ", which does not divide evenly into " + std::to_string(m.glyphs_per_row) + "x" +
                                 std::to_string(m.glyphs_per_col) + " glyph cells");

    font f;
    f.font_name = dir.filename().string();
    f.per_row = m.glyphs_per_row;
    f.per_col = m.glyphs_per_col;
    f.first_off = m.start_offset;
    f.cell_w = w / m.glyphs_per_row;
    f.cell_h = h / m.glyphs_per_col;
    f.atlas_w = w;

    const size_t n = (size_t)w * h;
    bool has_transparency = false;
    for (size_t i = 0; i < n && !has_transparency; i++) has_transparency = pixels.get()[i * 4 + 3] < 255;

    f.atlas.resize(n);
    for (size_t i = 0; i < n; i++) {
        const stbi_uc* p = pixels.get() + i * 4;
        if (has_transparency) f.atlas[i] = p[3] > 127;
        else                  f.atlas[i] = (p[0] * 299 + p[1] * 587 + p[2] * 114) / 1000 > 127;
    }
    return f;
}

}  