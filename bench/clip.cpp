// bench/clip.cpp
// Times full "clip + draw" for each line algorithm on integer-endpoint segments.
//
// Usage: bench_clip [buffer_size=2048] [segments=20000] [reps=11] [--no-png]
// Output: CSV on stdout, PNGs (clip_<workload>_<algo>.png) in the working directory.
#include "raster/frame.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>
#include <string>
#include <vector>

constexpr uint32_t BG = 0xff000000;   // frame's clear color
constexpr uint32_t FG = 0xffffffff;

using Seg = std::array<int, 4>;       // x0, y0, x1, y1  (all integers)
using DrawFn = void (g4d::frame::*)(int, int, int, int, uint32_t);

// Random integer segments. Start point uniform in [lo, hi]^2, direction uniform,
// length uniform in [1, maxlen], end point rounded to the nearest integer pixel.
std::vector<Seg> make_segments(int n, int lo, int hi, int maxlen, unsigned seed) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> P(lo, hi);
    std::uniform_real_distribution<double> A(0.0, 6.283185307179586);
    std::uniform_real_distribution<double> L(1.0, maxlen);
    std::vector<Seg> v(n);
    for (auto& s : v) {
        s[0] = P(rng);
        s[1] = P(rng);
        double a = A(rng), l = L(rng);
        s[2] = s[0] + (int)std::lround(l * std::cos(a));
        s[3] = s[1] + (int)std::lround(l * std::sin(a));
    }
    return v;
}

struct Result {
    double median_ms = 0;
    std::vector<uint32_t> image;      // frame contents after the last repetition
};

// Each repetition: clear (NOT timed) -> draw all segments (timed).
Result run(g4d::frame& fr, size_t npix, DrawFn draw, const std::vector<Seg>& segs, int reps) {
    using clk = std::chrono::steady_clock;
    std::vector<double> ms;

    for (int r = -2; r < reps; r++) {             // r < 0: warmup, discarded
        fr.clear();
        auto t0 = clk::now();
        for (const Seg& s : segs) (fr.*draw)(s[0], s[1], s[2], s[3], FG);
        auto t1 = clk::now();
        if (r >= 0) ms.push_back(std::chrono::duration<double, std::milli>(t1 - t0).count());
    }
    std::sort(ms.begin(), ms.end());

    Result res;
    res.median_ms = ms[ms.size() / 2];
    const uint32_t* b = fr.get_buffer();          // frame still holds the last rep
    res.image.assign(b, b + npix);
    return res;
}

size_t count_lit(const std::vector<uint32_t>& img) {
    return (size_t)std::count_if(img.begin(), img.end(), [](uint32_t p) { return p != BG; });
}
size_t count_diff(const std::vector<uint32_t>& a, const std::vector<uint32_t>& b) {
    size_t d = 0;
    for (size_t i = 0; i < a.size(); i++) d += (a[i] != b[i]);
    return d;
}

int main(int argc, char** argv) {
    int N = 2048, count = 20000, reps = 11;
    bool png = true;
    int pos = 0;
    for (int i = 1; i < argc; i++) {
        if (!std::strcmp(argv[i], "--no-png")) png = false;
        else if (pos == 0) { N = std::atoi(argv[i]); pos++; }
        else if (pos == 1) { count = std::atoi(argv[i]); pos++; }
        else if (pos == 2) { reps = std::atoi(argv[i]); pos++; }
    }

    g4d::frame fr(N, N);
    const size_t npix = (size_t)N * N;

    struct Workload { const char* name; std::vector<Seg> segs; };
    std::vector<Workload> loads;
    // all ranges are relative to the buffer size so the mix is the same at any N
    loads.push_back({"inside",   make_segments(count, N / 10,  N * 9 / 10, N / 30, 1)});  // short, fully visible
    loads.push_back({"mixed",    make_segments(count, -N / 2,  N * 3 / 2,  N / 6,  2)});  // some clipped, some not
    loads.push_back({"crossing", make_segments(count, -N / 2,  N * 3 / 2,  N * 6 / 5, 3)}); // long, mostly need clipping
    loads.push_back({"outside",  make_segments(count, -4 * N,  5 * N,      N / 30, 4)});  // nearly all rejected

    struct Algo { const char* name; DrawFn fn; };
    const Algo algos[] = {
        {"unclipped",        &g4d::frame::draw_line},          // baseline: iterates off-screen pixels too
        {"cohen_sutherland", &g4d::frame::draw_line_coh_sut},
        {"cyrus_beck",       &g4d::frame::draw_line_cyr_bec},
    };

    std::fprintf(stderr, "buffer %dx%d, %d segments per workload, %d reps\n", N, N, count, reps);
    std::puts("workload,algo,segments,median_ms,ns_per_segment,lit_pixels,pixels_differing_from_unclipped");

    for (auto& w : loads) {
        std::vector<uint32_t> reference;          // unclipped image = ground truth
        for (const Algo& a : algos) {
            Result r = run(fr, npix, a.fn, w.segs, reps);
            if (reference.empty()) reference = r.image;

            std::printf("%s,%s,%d,%.3f,%.1f,%zu,%zu\n", w.name, a.name, count, r.median_ms,
                        r.median_ms * 1e6 / count, count_lit(r.image),
                        count_diff(r.image, reference));

            if (png) {                            // written AFTER timing, never inside it
                std::string f = std::string("clip_") + w.name + "_" + a.name + ".png";
                stbi_write_png(f.c_str(), N, N, 4, r.image.data(), N * 4);
            }
        }
    }
}