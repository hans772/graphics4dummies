#include "util/font.h"
#include <fstream>
#include <sstream>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

namespace g4d {

font font::load_from_directory(const std::filesystem::path& font_dir) {

    std::ifstream manifest(font_dir / "manifest");

    if(!manifest) {
        std::cerr << "Error opening manifest file:" << std::endl;
        return {};
    }

    std::string glyph_name;
    int offset{-1};
    int rows{-1};
    int cols{-1};
    std::string linec;

    int found = 0;
    while(std::getline(manifest, linec)) {
        std::stringstream line(linec);
        std::string key;
        line >> key;
        if(!key.compare("[filename]")) {
            size_t b = linec.find_first_of('"');
            size_t e = linec.find_last_of('"');

            if(b != std::string::npos && e > b) {
                glyph_name = linec.substr(b+1, e - b - 1);
            }
            found |= 1;
        } else if(!key.compare("[start_offset]")) {
            line >> offset;
            found |= 2;
        } else if(!key.compare("[glyphs_per_row]")) {
            line >> rows;
            found |= 4;
        } else if(!key.compare("[glyphs_per_col]")) {
            line >> cols;
            found |= 8;
        }
    }

    if(found != 0b1111){
        std::cerr << "Invalid manifest" << std::endl;
        return {};
    }

    int width, height, channels;
    unsigned char* data = stbi_load((font_dir / glyph_name).string().c_str(), &width, &height, &channels, 4);


}

}