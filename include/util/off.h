#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <array>
#include <iomanip>
#include <algorithm>
#include <optional>
#include "util/color.h"

namespace g4d
{

    struct OFF_Mesh
    {
        std::vector<std::array<double, 3>> vertices{};        
        std::vector<std::vector<size_t>> polygons{};
        std::vector<color_t> colors{};        ///< Polygon / face colors (optional)
        std::vector<color_t> vertex_colors{}; ///< Vertex colors (optional)

        const std::vector<color_t>& polygon_colors() const { return colors; }
        std::vector<color_t>& polygon_colors() { return colors; }
    };

    namespace detail
    {
        /**
         * @brief Parses color values from extra tokens on an OFF vertex or polygon line.
         * Handles both integer (0..255) and float (0.0..1.0) representations with 3 (RGB) or 4 (RGBA) channels,
         * as well as optional leading component counts (e.g. "3 r g b" or "4 r g b a").
         */
        inline std::optional<color_t> parse_off_color(const std::vector<double>& vals)
        {
            if (vals.size() < 3)
                return std::nullopt;

            size_t offset = 0;
            // Check if first number is a count (3 or 4)
            if ((vals.size() == 4 && vals[0] == 3.0) || (vals.size() >= 5 && (vals[0] == 3.0 || vals[0] == 4.0)))
            {
                offset = 1;
            }

            if (vals.size() - offset < 3)
                return std::nullopt;

            double r = vals[offset + 0];
            double g = vals[offset + 1];
            double b = vals[offset + 2];
            double a = (vals.size() - offset >= 4) ? vals[offset + 3] : 255.0;

            // If all values are in 0.0 .. 1.0, scale up to 0 .. 255
            double max_c = std::max({r, g, b});
            if (vals.size() - offset >= 4)
            {
                max_c = std::max(max_c, a);
            }

            if (max_c <= 1.0 && max_c >= 0.0)
            {
                r *= 255.0;
                g *= 255.0;
                b *= 255.0;
                if (vals.size() - offset >= 4)
                {
                    a *= 255.0;
                }
                else
                {
                    a = 255.0;
                }
            }

            uint8_t ur = static_cast<uint8_t>(std::clamp(r, 0.0, 255.0));
            uint8_t ug = static_cast<uint8_t>(std::clamp(g, 0.0, 255.0));
            uint8_t ub = static_cast<uint8_t>(std::clamp(b, 0.0, 255.0));
            uint8_t ua = static_cast<uint8_t>(std::clamp(a, 0.0, 255.0));

            return color_t(ur, ug, ub, ua);
        }
    } // namespace detail

    /**
     * @brief Loads an OFF (.off / .coff) file into an OFF_Mesh.
     * Supports both face/polygon colors and vertex colors in integer (0-255) or float (0.0-1.0) format.
     * @param filename Path to the .off file.
     * @return OFF_Mesh containing vertices, polygons, and optional colors.
     */
    inline OFF_Mesh load_off(const std::string& filename)
    {
        OFF_Mesh mesh;
        std::ifstream in(filename);
        if (!in.is_open())
            return mesh;

        std::string line;
        // Skip comments or blank lines to find the header
        while (std::getline(in, line))
        {
            size_t first = line.find_first_not_of(" \t\r\n");
            if (first == std::string::npos || line[first] == '#')
                continue;
            break;
        }

        size_t off_pos = line.find("OFF");
        if (off_pos == std::string::npos)
            return mesh;

        size_t num_vertices = 0, num_faces = 0, num_edges = 0;
        // Header could have counts on the same line (e.g. "OFF 8 6 0")
        std::string rest = line.substr(off_pos + 3);
        size_t first_non_space = rest.find_first_not_of(" \t\r\n");
        if (first_non_space != std::string::npos)
        {
            std::istringstream iss(rest);
            iss >> num_vertices >> num_faces >> num_edges;
        }
        else
        {
            while (std::getline(in, line))
            {
                size_t first = line.find_first_not_of(" \t\r\n");
                if (first == std::string::npos || line[first] == '#')
                    continue;
                std::istringstream iss(line);
                iss >> num_vertices >> num_faces >> num_edges;
                break;
            }
        }

        mesh.vertices.reserve(num_vertices);
        mesh.vertex_colors.reserve(num_vertices);
        bool any_vertex_color = false;

        while (mesh.vertices.size() < num_vertices && std::getline(in, line))
        {
            size_t first = line.find_first_not_of(" \t\r\n");
            if (first == std::string::npos || line[first] == '#')
                continue;

            std::istringstream iss(line);
            double x = 0.0, y = 0.0, z = 0.0;
            if (iss >> x >> y >> z)
            {
                mesh.vertices.push_back({x, y, z});

                std::vector<double> extra;
                double val;
                while (iss >> val)
                {
                    extra.push_back(val);
                }

                auto col = detail::parse_off_color(extra);
                if (col)
                {
                    mesh.vertex_colors.push_back(*col);
                    any_vertex_color = true;
                }
                else
                {
                    mesh.vertex_colors.push_back(color_t(200, 200, 200, 255));
                }
            }
        }

        if (!any_vertex_color)
        {
            mesh.vertex_colors.clear();
        }

        mesh.polygons.reserve(num_faces);
        mesh.colors.reserve(num_faces);
        bool any_face_color = false;

        while (mesh.polygons.size() < num_faces && std::getline(in, line))
        {
            size_t first = line.find_first_not_of(" \t\r\n");
            if (first == std::string::npos || line[first] == '#')
                continue;

            std::istringstream iss(line);
            size_t n = 0;
            if (!(iss >> n) || n < 3)
                continue;

            std::vector<size_t> poly(n);
            for (size_t i = 0; i < n; ++i)
            {
                iss >> poly[i];
            }
            mesh.polygons.push_back(std::move(poly));

            std::vector<double> extra;
            double val;
            while (iss >> val)
            {
                extra.push_back(val);
            }

            auto col = detail::parse_off_color(extra);
            if (col)
            {
                mesh.colors.push_back(*col);
                any_face_color = true;
            }
            else
            {
                mesh.colors.push_back(color_t(200, 200, 200, 255));
            }
        }

        if (!any_face_color)
        {
            mesh.colors.clear();
        }

        return mesh;
    }

    inline OFF_Mesh off_read(const char *filename)
    {
        return load_off(filename);
    }

    inline void off_write(const char *filename, const OFF_Mesh& mesh)
    {
        std::ofstream out(filename);
        if (!out)
            return;

        bool has_poly_colors = (!mesh.colors.empty() && mesh.colors.size() == mesh.polygons.size());
        bool has_vertex_colors = (!mesh.vertex_colors.empty() && mesh.vertex_colors.size() == mesh.vertices.size());
        bool write_vertex_colors = has_vertex_colors && !has_poly_colors;

        if (write_vertex_colors)
            out << "COFF\n";
        else
            out << "OFF\n";

        size_t vert{mesh.vertices.size()}, edge{0}, poly{mesh.polygons.size()};
        out << vert << ' ' << poly << ' ' << edge << '\n';

        for (size_t v{0}; v < vert; v++)
        {
            out << std::setprecision(7) << mesh.vertices[v][0] << ' ' 
                << std::setprecision(7) << mesh.vertices[v][1] << ' ' 
                << std::setprecision(7) << mesh.vertices[v][2];
            if (write_vertex_colors)
            {
                out << ' ' << static_cast<int>(mesh.vertex_colors[v].red)
                    << ' ' << static_cast<int>(mesh.vertex_colors[v].green)
                    << ' ' << static_cast<int>(mesh.vertex_colors[v].blue)
                    << ' ' << static_cast<int>(mesh.vertex_colors[v].alpha);
            }
            out << '\n';  
        }

        for (size_t p = 0; p < poly; ++p)                      
        {
            const auto &pg = mesh.polygons[p];
            size_t pv = pg.size();
            edge += pv;
            out << pv;

            for (auto &vx : pg)
            {
                out << ' ' << vx;
            }
            if (has_poly_colors)
            {
                out << ' ' << static_cast<int>(mesh.colors[p].red)
                    << ' ' << static_cast<int>(mesh.colors[p].green)
                    << ' ' << static_cast<int>(mesh.colors[p].blue)
                    << ' ' << static_cast<int>(mesh.colors[p].alpha);
            }
            out << '\n';
        }

        out.flush();
        out.close();
    }

    inline void off_write(const std::string& filename, const OFF_Mesh& mesh)
    {
        off_write(filename.c_str(), mesh);
    }

};
