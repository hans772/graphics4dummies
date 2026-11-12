#pragma once

#include <fstream>
#include <vector>
#include <array>
#include <iomanip>

namespace g4d
{

    struct OFF_Mesh
    {
        std::vector<std::array<double, 3>> vertices;        
        std::vector<std::vector<size_t>> polygons;
    };

    inline void off_write(const char *filename, OFF_Mesh mesh)
    {
        std::ofstream out(filename);
        if (!out)
            return;

        out << "OFF\n";

        size_t vert{mesh.vertices.size()}, edge{0}, poly{mesh.polygons.size()};
        out << vert << ' ' << poly << ' ' << edge << '\n';

        for (int v{0}; v < vert; v++)
        {
            out << std::setprecision(7) << mesh.vertices[v][0] << ' ' 
                << std::setprecision(7) << mesh.vertices[v][1] << ' ' 
                << std::setprecision(7) << mesh.vertices[v][2] << '\n';  
        }

        for (auto &pg : mesh.polygons)                      
        {
            size_t pv = pg.size();
            edge += pv;
            out << pv;

            for (auto &vx : pg)
            {
                out << ' ' << vx;
            }
            out << '\n';
        }

        out.flush();
        out.close();
    }
};