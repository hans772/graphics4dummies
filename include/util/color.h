#pragma once

#include <cstdint>
#include <bit>

namespace g4d
{

    struct color_t
    {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
        uint8_t alpha;

        constexpr color_t(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : red(r), green(g), blue(b), alpha(a) {}
        constexpr color_t(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b), alpha(255) {}
        constexpr color_t(uint8_t grey) : red(grey), green(grey), blue(grey), alpha(255) {}

        constexpr uint32_t get() const { 
            return color_t::rgba(red,green,blue,alpha); 
        };
        
        static constexpr color_t from_hex(uint32_t hex_rgba) {
            int8_t r = (hex_rgba >> 24) & 0xFF;
            uint8_t g = (hex_rgba >> 16) & 0xFF;
            uint8_t b = (hex_rgba >> 8) & 0xFF;
            uint8_t a = (hex_rgba) & 0xFF;

            return color_t(r,g,b,a);
        }

        static constexpr uint32_t rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
        {
            if constexpr (std::endian::native == std::endian::little)
            {
                return (uint32_t(a) << 24) |
                       (uint32_t(b) << 16) |
                       (uint32_t(g) << 8) |
                       (uint32_t(r));
            }
            else
            {
                return (uint32_t(r) << 24) |
                       (uint32_t(g) << 16) |
                       (uint32_t(b) << 8) |
                       (uint32_t(a));
            }
        }

        static constexpr uint32_t hex(uint32_t hex_rgba)
        {
            uint8_t r = (hex_rgba >> 24) & 0xFF;
            uint8_t g = (hex_rgba >> 16) & 0xFF;
            uint8_t b = (hex_rgba >> 8) & 0xFF;
            uint8_t a = (hex_rgba) & 0xFF;
            return rgba(r, g, b, a);
        }

        bool operator==(const color_t& right) const {
            return get() == right.get();
        }
    };

    inline constexpr color_t G4D_COLOR_WHITE = color_t(255, 255, 255, 255);
    inline constexpr color_t G4D_COLOR_RED = color_t(255, 0, 0, 255);
    inline constexpr color_t G4D_COLOR_GREEN = color_t(0, 255, 0, 255);
    inline constexpr color_t G4D_COLOR_BLUE = color_t(0, 0, 255, 255);
    inline constexpr color_t G4D_COLOR_BLACK = color_t(0, 0, 0, 255);

} // namespace g4d