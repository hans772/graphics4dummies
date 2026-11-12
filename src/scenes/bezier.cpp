#include "scene/bezier.h"
#include <algorithm>
#include <cstdio>
#include <string>
#include "glm/gtc/matrix_transform.hpp"
#include "util/off.h"

namespace g4d {

namespace {

const char* tool_to_string(bezier_scene::tool_mode m) {
    switch (m) {
        case bezier_scene::tool_mode::add:  return "ADD (Click to insert point)";
        case bezier_scene::tool_mode::edit: return "EDIT (Click & drag point)";
        case bezier_scene::tool_mode::del:  return "DELETE (Click point to remove)";
        case bezier_scene::tool_mode::pan:  return "PAN (Drag to pan view)";
    }
    return "UNKNOWN";
}

void revolve_bezier(const std::vector<glm::vec2>& points, int num_sections = 20) {
    std::vector<glm::vec4> h_points;
    h_points.reserve(points.size());

    for(auto& pt : points) h_points.emplace_back(pt, 0.0f, 1.0f);

    if(points.back() == points.front()) {
        // revolve a closed loop
    }
    else {

        glm::vec3 n{glm::normalize(h_points.back() - h_points.front())};
        glm::vec3 k{0.0f, 0.0f, 1.0f};
        glm::vec3 u{glm::normalize(glm::cross(n, k))};
        glm::vec3 v{glm::normalize(glm::cross(n, u))};

        glm::mat4x4 axis_align{{u, 0.f}, {v, 0.f}, {n, 0.f}, {0.f, 0.f, 0.f, 1.f}};
        glm::mat4x4 translate{glm::translate(glm::mat4x4(1.0f), -glm::vec3(h_points.front()))};

        double sec_step = ((double)360)/num_sections;

        glm::mat4x4 M{glm::transpose(axis_align) * translate};
        glm::mat4x4 R{glm::rotate(glm::mat4(1.0f), (float)glm::radians(sec_step), glm::vec3(0.0f, 0.0f, 1.0f))};
        
        OFF_Mesh mesh;
        mesh.vertices.reserve(points.size()*num_sections);
        mesh.polygons.reserve((points.size()-1)*num_sections);
        
        std::vector<glm::vec4> o_points(points.size());
        std::vector<glm::vec4> n_points(points.size());

        for(int i = 0; i < points.size(); i++) {
            n_points[i] = M*h_points[i];
        }
        mesh.vertices.push_back({n_points[0].x, n_points[0].y, n_points[0].z});
        mesh.vertices.push_back({n_points.back().x, n_points.back().y, n_points.back().z});

        for(int i = 1; i < points.size()-1; i++) {
            mesh.vertices.push_back({n_points[i].x, n_points[i].y, n_points[i].z});
        }

        std::swap(n_points, o_points);

        auto connect_layers = [&](){
            int tot = mesh.vertices.size();
            int fin = tot - points.size() + 2;
            int fio = fin - points.size() + 2;
            mesh.polygons.push_back({0, fin, fio});
            mesh.polygons.push_back({tot-1, 1, fin-1});
            for(int i = 0; i < points.size()-3; i++) {
                mesh.polygons.push_back({fin + i, fin + i+1, fio + i + 1, fio + i});
            }
        };

        for(int sec = 1; sec < num_sections; sec++) {
            for(int i = 1; i < points.size()-1; i++) {
                n_points[i] = R * o_points[i];
                mesh.vertices.push_back({n_points[i].x, n_points[i].y, n_points[i].z});
            }
            connect_layers();
            std::swap(n_points, o_points);
        }

        int fin = 2;
        int fio = mesh.vertices.size()-points.size()+2;
        mesh.polygons.push_back({0, fin, fio});
        mesh.polygons.push_back({points.size()-1, 1, mesh.vertices.size()-1});

        for(int i = 0; i < points.size()-3; i++) {
            mesh.polygons.push_back({fin + i, fin + i+1, fio + i + 1, fio + i});
        }

        off_write("bezier_revolve.off", mesh);
    }
    

}

}  // namespace

bezier_scene::bezier_scene(const std::string_view& fonts_dir)
    : f(font::load(fonts_dir, "kitchen_sink")),
      tex_r(f),
      curve(glm::vec2(150.0f, 250.0f), glm::vec2(650.0f, 250.0f)) {
    // Start with a smooth 4-point curve
    curve.add_point(glm::vec2(300.0f, 500.0f));
    curve.add_point(glm::vec2(500.0f, 100.0f));
    curve.set_tolerance(tolerance, true);
}

void bezier_scene::update(double /*dt*/) {}

int bezier_scene::find_near_point(glm::ivec2 screen_pos, float threshold) const {
    const auto& pts = curve.control_points();
    int idx = 0;
    int closest_idx = -1;
    float min_dist = threshold;

    for (const auto& pt : pts) {
        glm::ivec2 sp = world_to_screen(pt);
        float d = glm::distance(glm::vec2(screen_pos), glm::vec2(sp));
        if (d < min_dist) {
            min_dist = d;
            closest_idx = idx;
        }
        idx++;
    }
    return closest_idx;
}

void bezier_scene::render(frame& fr) {
    const auto& pts = curve.control_points();

    // 1. Draw Control Polygon
    if (pts.size() >= 2) {
        auto it = pts.begin();
        auto next_it = std::next(it);
        while (next_it != pts.end()) {
            glm::ivec2 p1 = world_to_screen(*it);
            glm::ivec2 p2 = world_to_screen(*next_it);
            draw_line_cyr_bec(fr, p1.x, p1.y, p2.x, p2.y, color_t(70, 70, 95, 255));
            it++;
            next_it++;
        }
    }

    // 2. Draw Bezier Curve
    if (rmode == render_mode::subdivision) {
        curve.draw(fr, pan_offset.x, pan_offset.y, subdivisions, color_t(80, 200, 255, 255));
    } else {
        curve.draw_approx(fr, pan_offset.x, pan_offset.y, color_t(100, 255, 150, 255));
    }

    // 3. Draw Control Points
    int idx = 0;
    int total = static_cast<int>(pts.size());
    for (const auto& pt : pts) {
        glm::ivec2 sp = world_to_screen(pt);

        // Different colors for start, end, and intermediate control points
        color_t pt_col(255, 215, 0, 255);  // Yellow
        size_t r = 5;
        if (idx == 0) {
            pt_col = color_t(50, 255, 120, 255);  // Green
            r = 6;
        } else if (idx == total - 1) {
            pt_col = color_t(255, 80, 80, 255);   // Red
            r = 6;
        }

        draw_circle(fr, sp.x, sp.y, r, pt_col);

        // Selection / Hover highlight ring
        if (idx == hovered_point || idx == selected_point) {
            draw_circle(fr, sp.x, sp.y, 9, color_t(255, 255, 255, 255));
        }

        // Draw index label next to point
        tex_r.draw(fr, sp.x + 8, sp.y + 8, std::to_string(idx),
                   {.text_color = color_t(200, 200, 200, 255),
                    .scale = 1,
                    .letter_spacing = 0,
                    .line_spacing = 0,
                    .background = std::nullopt});

        idx++;
    }

    // 4. On-Screen HUD / Help Overlay
    const int top_y = static_cast<int>(fr.height()) - 18;

    // Line 1: Tool Mode
    tex_r.draw(fr, 15, top_y,
               "Mode: " + std::string(tool_to_string(tool)) + "  [TAB: cycle | 1:Add 2:Edit 3:Del 4:Pan]",
               {.text_color = color_t(255, 255, 255, 255),
                .scale = 2,
                .letter_spacing = 0,
                .line_spacing = 0,
                .background = std::nullopt});

    // Line 2: Render Method & Parameters
    if (rmode == render_mode::subdivision) {
        std::string s = "Method: SUBDIVISION (N = " + std::to_string(subdivisions) +
                        ")  [SPACE: toggle | Scroll: adjust N]";
        tex_r.draw(fr, 15, top_y - 24, s,
                   {.text_color = color_t(80, 200, 255, 255),
                    .scale = 2,
                    .letter_spacing = 0,
                    .line_spacing = 0,
                    .background = std::nullopt});
    } else {
        char buf[128];
        std::snprintf(buf, sizeof(buf),
                      "Method: TOLERANCE (Tol = %.3f)  [SPACE: toggle | Scroll: adjust Tol]",
                      tolerance);
        tex_r.draw(fr, 15, top_y - 24, buf,
                   {.text_color = color_t(100, 255, 150, 255),
                    .scale = 2,
                    .letter_spacing = 0,
                    .line_spacing = 0,
                    .background = std::nullopt});
    }

    // Line 3: Info & Navigation
    std::string info = "Points: " + std::to_string(total) +
                       " | Pan: (" + std::to_string(pan_offset.x) + ", " +
                       std::to_string(pan_offset.y) + ")  [Right-drag to pan | R: reset]";
    tex_r.draw(fr, 15, top_y - 48, info,
               {.text_color = color_t(180, 180, 180, 255),
                .scale = 1,
                .letter_spacing = 0,
                .line_spacing = 0,
                .background = std::nullopt});
}

void bezier_scene::on_key(int key, int action) {
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;

    if (action == GLFW_PRESS) {
        // Tab cycles tool modes
        if (key == GLFW_KEY_TAB) {
            switch (tool) {
                case tool_mode::add:  tool = tool_mode::edit; break;
                case tool_mode::edit: tool = tool_mode::del;  break;
                case tool_mode::del:  tool = tool_mode::pan;  break;
                case tool_mode::pan:  tool = tool_mode::add;  break;
            }
            is_dragging_point = false;
            is_panning = false;
        }

        // Direct tool mode selection
        if (key == GLFW_KEY_1) tool = tool_mode::add;
        if (key == GLFW_KEY_2) tool = tool_mode::edit;
        if (key == GLFW_KEY_3) tool = tool_mode::del;
        if (key == GLFW_KEY_4) tool = tool_mode::pan;

        // Toggle render mode
        if (key == GLFW_KEY_SPACE || key == GLFW_KEY_M) {
            rmode = (rmode == render_mode::subdivision) ? render_mode::tolerance
                                                        : render_mode::subdivision;
        }
        if (key == GLFW_KEY_S) rmode = render_mode::subdivision;
        if (key == GLFW_KEY_T) rmode = render_mode::tolerance;

        // Reset pan
        if (key == GLFW_KEY_R) {
            pan_offset = glm::ivec2(0, 0);
        }

        if (key == GLFW_KEY_ENTER) {
            revolve_bezier(curve.sample_line(0.05), 45);
        }
    }

    // Parameter adjustment via keyboard (+/- or Up/Down)
    if (key == GLFW_KEY_UP || key == GLFW_KEY_EQUAL) {
        on_scroll(0.0, 1.0);
    } else if (key == GLFW_KEY_DOWN || key == GLFW_KEY_MINUS) {
        on_scroll(0.0, -1.0);
    }
}

void bezier_scene::on_mouse_move(int x, int y) {
    mouse_pos = glm::ivec2(x, y);

    // Update hovered point when not dragging
    if (!is_dragging_point && !is_panning) {
        hovered_point = find_near_point(mouse_pos, 16.0f);
    }

    // Handle point dragging in edit mode
    if (is_dragging_point && selected_point >= 0) {
        glm::vec2 world_p = screen_to_world(mouse_pos);
        curve.reposition_point(static_cast<size_t>(selected_point), world_p, true);
    }

    // Handle viewport panning
    if (is_panning) {
        pan_offset = pan_base + (mouse_pos - pan_anchor);
    }
}

void bezier_scene::on_mouse_button(int button, int action, int x, int y) {
    mouse_pos = glm::ivec2(x, y);

    // Right-click drag pans universally across all modes
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            is_panning = true;
            pan_anchor = mouse_pos;
            pan_base = pan_offset;
        } else if (action == GLFW_RELEASE) {
            is_panning = false;
        }
        return;
    }

    if (button != GLFW_MOUSE_BUTTON_LEFT) return;

    if (action == GLFW_PRESS) {
        switch (tool) {
            case tool_mode::add: {
                glm::vec2 world_p = screen_to_world(mouse_pos);
                curve.add_point(world_p, true);
                hovered_point = find_near_point(mouse_pos, 16.0f);
                break;
            }
            case tool_mode::edit: {
                int hit = find_near_point(mouse_pos, 16.0f);
                if (hit != -1) {
                    selected_point = hit;
                    is_dragging_point = true;
                }
                break;
            }
            case tool_mode::del: {
                int hit = find_near_point(mouse_pos, 16.0f);
                if (hit != -1) {
                    curve.delete_point(static_cast<size_t>(hit), true);
                    hovered_point = find_near_point(mouse_pos, 16.0f);
                }
                break;
            }
            case tool_mode::pan: {
                is_panning = true;
                pan_anchor = mouse_pos;
                pan_base = pan_offset;
                break;
            }
        }
    } else if (action == GLFW_RELEASE) {
        is_dragging_point = false;
        selected_point = -1;
        if (tool == tool_mode::pan) {
            is_panning = false;
        }
    }
}

void bezier_scene::on_scroll(double /*xoffset*/, double yoffset) {
    if (rmode == render_mode::subdivision) {
        if (yoffset > 0) {
            subdivisions += (subdivisions >= 50 ? 5 : 1);
        } else if (yoffset < 0) {
            if (subdivisions > 1) {
                subdivisions -= (subdivisions > 50 ? 5 : 1);
            }
        }
        subdivisions = std::clamp<size_t>(subdivisions, 1, 500);
    } else {
        // In tolerance mode: positive scroll refines (smaller tolerance), negative scroll coarsens
        if (yoffset > 0) {
            tolerance = std::max(0.01, tolerance * 0.85);
        } else if (yoffset < 0) {
            tolerance = std::min(100.0, tolerance * 1.18);
        }
        curve.set_tolerance(tolerance, true);
    }
}

}  // namespace g4d
