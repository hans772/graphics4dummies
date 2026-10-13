#include <glad/glad.h>

#include "render/renderer.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace g4d {

namespace {

// g4d colours are 0xAARRGGBB which is BGRA in little endian machines
constexpr GLenum PIXEL_FORMAT = GL_BGRA;

bool read_file(const std::string& path, std::string& out) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        std::fprintf(stderr, "cannot open shader file: %s\n", path.c_str());
        return false;
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    out = ss.str();
    return true;
}

GLuint compile_stage(GLenum type, const std::string& src, const char* name) {
    GLuint s = glCreateShader(type);
    const char* c = src.c_str();
    glShaderSource(s, 1, &c, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[2048];
        glGetShaderInfoLog(s, sizeof log, nullptr, log);
        std::fprintf(stderr, "%s: compile error:\n%s\n", name, log);
        glDeleteShader(s);
        return 0;
    }
    return s;
}

GLuint build_program(const std::string& dir) {
    std::string vsrc, fsrc;
    if (!read_file(dir + "/screen.vert", vsrc) || !read_file(dir + "/screen.frag", fsrc))
        return 0;

    GLuint vs = compile_stage(GL_VERTEX_SHADER, vsrc, "screen.vert");
    GLuint fs = compile_stage(GL_FRAGMENT_SHADER, fsrc, "screen.frag");
    if (!vs || !fs) {
        glDeleteShader(vs);   
        glDeleteShader(fs);
        return 0;
    }

    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[2048];
        glGetProgramInfoLog(p, sizeof log, nullptr, log);
        std::fprintf(stderr, "shader link error:\n%s\n", log);
        glDeleteProgram(p);
        return 0;
    }

    glUseProgram(p);
    glUniform1i(glGetUniformLocation(p, "tex"), 0); 
    return p;
}

}  // namespace

renderer::renderer(int width, int height, std::string dir)
    : w(width), h(height), shader_dir(std::move(dir)) {
    program = build_program(shader_dir);
    if (!program) throw std::runtime_error("could not build shaders from '" + shader_dir + "'");

    glGenVertexArrays(1, &vao); 
    allocate_texture();
}

renderer::~renderer() {
    glDeleteTextures(1, &tex);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(program);
}

void renderer::allocate_texture() {
    if (tex) glDeleteTextures(1, &tex);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, PIXEL_FORMAT, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);  
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

bool renderer::reload_shaders() {
    GLuint p = build_program(shader_dir);
    if (!p) return false;
    glDeleteProgram(program);
    program = p;
    std::fprintf(stderr, "shaders reloaded\n");
    return true;
}

void renderer::resize(int width, int height) {
    if (width == w && height == h) return;
    w = width;
    h = height;
    allocate_texture();
}

void renderer::upload(const uint32_t* pixels) {
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, PIXEL_FORMAT, GL_UNSIGNED_BYTE, pixels);
}

void renderer::present(int fb_w, int fb_h) {
    if (fb_w <= 0 || fb_h <= 0) return;   // minimized
    last_fb_h = fb_h;

    glViewport(0, 0, fb_w, fb_h);
    glClearColor(bg[0], bg[1], bg[2], 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // fit with aspect ratio kept; integer scale factors when larger than the buffer, for crisp pixels
    float s = std::min((float)fb_w / w, (float)fb_h / h);
    if (s >= 1.0f) s = std::floor(s);
    vw = std::max(1, (int)(w * s));
    vh = std::max(1, (int)(h * s));
    vx = (fb_w - vw) / 2;
    vy = (fb_h - vh) / 2;
    glViewport(vx, vy, vw, vh);

    glUseProgram(program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

bool renderer::screen_to_pixel(double sx, double sy, int& px, int& py) const {
    double gx = sx - vx;
    double gy = (last_fb_h - sy) - vy;    // GL origin is bottom-left, cursor origin is top-left
    if (gx < 0 || gy < 0 || gx >= vw || gy >= vh) return false;
    px = std::min((int)(gx * w / vw), w - 1);
    py = std::min((int)(gy * h / vh), h - 1);
    return true;
}

}  // namespace g4d