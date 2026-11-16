#pragma once

#include <cstdint>
#include <bit>
#include <algorithm>

namespace g4d
{

    struct color_t;

    namespace color
    {
        inline constexpr uint32_t rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
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

        inline constexpr uint32_t hex(uint32_t hex_rgba)
        {
            uint8_t r = (hex_rgba >> 24) & 0xFF;
            uint8_t g = (hex_rgba >> 16) & 0xFF;
            uint8_t b = (hex_rgba >> 8) & 0xFF;
            uint8_t a = (hex_rgba) & 0xFF;
            return rgba(r, g, b, a);
        }
    } // namespace color

    struct color_t
    {
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        uint8_t alpha = 255;

        constexpr color_t() = default;
        constexpr color_t(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
            : red(r), green(g), blue(b), alpha(a) {}
        constexpr color_t(uint8_t grey)
            : red(grey), green(grey), blue(grey), alpha(255) {}

        constexpr uint32_t get() const
        {
            return color::rgba(red, green, blue, alpha);
        }

        bool operator==(const color_t &other) const
        {
            return get() == other.get();
        }
    };

    namespace color
    {
        inline constexpr color_t from_hex(uint32_t hex_rgba)
        {
            uint8_t r = (hex_rgba >> 24) & 0xFF;
            uint8_t g = (hex_rgba >> 16) & 0xFF;
            uint8_t b = (hex_rgba >> 8) & 0xFF;
            uint8_t a = (hex_rgba) & 0xFF;
            return color_t(r, g, b, a);
        }

        inline constexpr color_t lerp(const color_t &c1, const color_t &c2, float t)
        {
            auto mix_channel = [t](uint8_t a, uint8_t b) -> uint8_t
            {
                float val = static_cast<float>(a) + (static_cast<float>(b) - static_cast<float>(a)) * t;
                return static_cast<uint8_t>(std::clamp(val, 0.0f, 255.0f));
            };

            return color_t(
                mix_channel(c1.red, c2.red),
                mix_channel(c1.green, c2.green),
                mix_channel(c1.blue, c2.blue),
                mix_channel(c1.alpha, c2.alpha));
        }
    }

    inline constexpr color_t G4D_COLOR_WHITE = color_t(255, 255, 255, 255);
    inline constexpr color_t G4D_COLOR_RED = color_t(255, 0, 0, 255);
    inline constexpr color_t G4D_COLOR_GREEN = color_t(0, 255, 0, 255);
    inline constexpr color_t G4D_COLOR_BLUE = color_t(0, 0, 255, 255);
    inline constexpr color_t G4D_COLOR_BLACK = color_t(0, 0, 0, 255);

} // namespace g4d