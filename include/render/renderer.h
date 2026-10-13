#pragma once

#include <cstdint>
#include <string>

namespace g4d {

/**
 * @brief shows a CPU pixel buffer on screen using one texture and a fullscreen triangle.
 *
 * Owns the shader program, VAO and texture. Needs a current OpenGL 3.3 context
 * (create a g4d::window first). Does no rasterization itself.
 */
class renderer {
public:
    /**
     * @param width       Width of the pixel buffer that will be uploaded
     * @param height      Height of the pixel buffer that will be uploaded
     * @param shader_dir  Directory containing screen.vert and screen.frag shader
     * @throws std::runtime_error if the shaders cannot be loaded or compiled
     */
    renderer(int width, int height, std::string shader_dir);
    ~renderer();

    renderer(const renderer&) = delete;
    renderer& operator=(const renderer&) = delete;

    /**
     * @brief recompiles the shader files.
     * @return false on failure (error is printed to stderr and the old shaders stay active)
     */
    bool reload_shaders();

    /** @brief Changes the pixel buffer resolution (reallocates the texture). */
    void resize(int width, int height);

    /**
     * @brief Copies width*height pixels (0xAARRGGBB, row 0 = top) into the texture.
     */
    void upload(const uint32_t* pixels);

    /**
     * @brief Draws the texture into the window, scaled to fit with the aspect ratio kept.
     * @param fb_width   Current window framebuffer width in pixels
     * @param fb_height  Current window framebuffer height in pixels
     */
    void present(int fb_width, int fb_height);

    /**
     * @brief Converts a position in window framebuffer pixels (origin top-left)
     *        to a pixel of the rasterizer buffer (origin bottom-left, y up).
     * @return false if the position is outside the displayed image (valid after present())
     */
    bool screen_to_pixel(double sx, double sy, int& px, int& py) const;

    /** @brief Colour of the bars around the image when the window aspect ratio differs. */
    void set_background(float r, float g, float b) { bg[0] = r; bg[1] = g; bg[2] = b; }

    int width() const { return w; }
    int height() const { return h; }

private:
    void allocate_texture();

    unsigned int program = 0, vao = 0, tex = 0;
    int w, h;
    std::string shader_dir;
    float bg[3] = {0.08f, 0.08f, 0.08f};

    // placement of the image in the last presented frame
    int vx = 0, vy = 0, vw = 1, vh = 1, last_fb_h = 1;
};

}  // namespace g4d