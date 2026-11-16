#pragma once

#include "raster/3d/depth_buffer.h"
#include "geometry/vertex3d.h"
#include "raster/frame.h"

#include <vector>
#include <array>
#include <algorithm>

namespace g4d
{
    namespace detail {
        inline clip_vertex vec_lerp(const clip_vertex& v0, const clip_vertex& v1, float t) {
            return {
                glm::mix(v0.position, v1.position, t),
                glm::normalize(glm::mix(v0.normal, v1.normal, t)),
                color::lerp(v0.color, v1.color, t)
            };
        }

        inline float edge_eval(glm::vec2 a, glm::vec2 b, glm::vec2 c) {
            return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
        }

    }

    namespace triangle
    {

        inline void raster(frame &f, depth_buffer &db, const screen_vertex &p1, const screen_vertex &p2, const screen_vertex &p3) {
            int min_x = std::max(0, static_cast<int>(std::min({p1.position.x, p2.position.x, p3.position.x})));
            int min_y = std::max(0, static_cast<int>(std::min({p1.position.y, p2.position.y, p3.position.y})));
            int max_x = std::min(static_cast<int>(f.width() - 1), static_cast<int>(std::max({p1.position.x, p2.position.x, p3.position.x})));
            int max_y = std::min(static_cast<int>(f.height() - 1), static_cast<int>(std::max({p1.position.y, p2.position.y, p3.position.y})));

            float area = detail::edge_eval(p1.position, p2.position, p3.position);
            if (area <= 0.0f) return;

            float inv_area = 1.0f / area;

            for (int y = min_y; y <= max_y; ++y) {
                for (int x = min_x; x <= max_x; ++x) {
                    glm::vec2 p(x + 0.5f, y + 0.5f);

                    float w0 = detail::edge_eval(p2.position, p3.position, p);
                    float w1 = detail::edge_eval(p3.position, p1.position, p);
                    float w2 = detail::edge_eval(p1.position, p2.position, p);

                    if (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f) {
                        float l0 = w0 * inv_area;
                        float l1 = w1 * inv_area;
                        float l2 = w2 * inv_area;

                        float inv_w = l0 * p1.invw + l1 * p2.invw + l2 * p3.invw;
 
                        if (db.test_and_set(x, y, inv_w)) {
                            float w_curr = (inv_w > 0.0f) ? (1.0f / inv_w) : 1.0f;
                            float r = (l0 * p1.color.red * p1.invw + l1 * p2.color.red * p2.invw + l2 * p3.color.red * p3.invw) * w_curr;
                            float g = (l0 * p1.color.green * p1.invw + l1 * p2.color.green * p2.invw + l2 * p3.color.green * p3.invw) * w_curr;
                            float b = (l0 * p1.color.blue * p1.invw + l1 * p2.color.blue * p2.invw + l2 * p3.color.blue * p3.invw) * w_curr;
                            color_t pix_color(
                                static_cast<uint8_t>(std::clamp(r, 0.0f, 255.0f)),
                                static_cast<uint8_t>(std::clamp(g, 0.0f, 255.0f)),
                                static_cast<uint8_t>(std::clamp(b, 0.0f, 255.0f)),
                                255
                            );
                            f.draw_pixel(x, y, pix_color);
                        }
                    }
                }
            }
        };

        inline std::vector<std::array<clip_vertex, 3>> clip(const clip_vertex &p1, const clip_vertex &p2, const  clip_vertex &p3, float znear = 0.1f)
        {
            
            std::vector<std::array<clip_vertex, 3>> res;
            
            int out = 0;
            std::array<const clip_vertex *, 3> vertices{&p1, &p2, &p3};
            std::array<bool, 3> outvex{false, false, false};
            for (int i = 0; i < 3; i++)
            {
                if (vertices[i]->position.w >= znear) continue;
                else { outvex[i] = true; out++; }
            }

            if(out==0) {
                res.push_back({p1, p2, p3});
            } else if (out == 1) {
                int out_i = outvex[0] ? 0 : (outvex[1] ? 1 : 2);
                const clip_vertex* out_v = vertices[out_i];
                const clip_vertex* in_v1 = vertices[(out_i+1)%3];
                const clip_vertex* in_v2 = vertices[(out_i+2)%3];

                float t1 = (znear - out_v->position.w)/(in_v1->position.w - out_v->position.w);
                float t2 = (znear - out_v->position.w)/(in_v2->position.w - out_v->position.w);

                clip_vertex i1{detail::vec_lerp(*out_v, *in_v1, t1)};
                clip_vertex i2{detail::vec_lerp(*out_v, *in_v2, t2)};

                res.push_back({i1, *in_v1, *in_v2});
                res.push_back({*in_v2, i2, i1});                
            } else if (out == 2) {
                int in_i = !outvex[0] ? 0 : (!outvex[1] ? 1 : 2);
                const clip_vertex* in_v = vertices[in_i];
                const clip_vertex* out_v1 = vertices[(in_i+1)%3];
                const clip_vertex* out_v2 = vertices[(in_i+2)%3];

                float t1 = (znear - in_v->position.w)/(out_v1->position.w - in_v->position.w);
                float t2 = (znear - in_v->position.w)/(out_v2->position.w - in_v->position.w);

                clip_vertex i1{detail::vec_lerp(*in_v, *out_v1, t1)};
                clip_vertex i2{detail::vec_lerp(*in_v, *out_v2, t2)};

                res.push_back({*in_v, i1, i2});
            }

            return res;
        }

    };

} // namespace g4d
