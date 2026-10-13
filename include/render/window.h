#pragma once

#include <functional>
#include <string>

struct GLFWwindow;

namespace g4d {

/**
 * @brief GLFW window with an OpenGL 3.3 core context, with glad already initialised.
 *
 * Construct this before a g4d::renderer. Not copyable or movable.
 */
class window {
public:
    /** @throws std::runtime_error if GLFW or the GL context cannot be created */
    window(int width, int height, const std::string& title);
    ~window();

    window(const window&) = delete;
    window& operator=(const window&) = delete;

    bool should_close() const;
    void close();
    void poll_events();
    void swap_buffers();
    void set_title(const std::string& title);
    void set_vsync(bool on);

    /** @brief Seconds since GLFW started. */
    double time() const;

    /** @brief Size of the drawable area in pixels (differs from window size on HiDPI). */
    void framebuffer_size(int& w, int& h) const;

    /** @brief Cursor position in framebuffer pixels, origin top-left. */
    void cursor_position(double& x, double& y) const;

    /** Called with (GLFW_KEY_*, GLFW_PRESS / GLFW_RELEASE / GLFW_REPEAT). */
    std::function<void(int key, int action)> on_key;

    /** Called with (GLFW_MOUSE_BUTTON_*, GLFW_PRESS / GLFW_RELEASE). */
    std::function<void(int button, int action)> on_mouse_button;

private:
    GLFWwindow* handle = nullptr;
};

}  // namespace g4d