#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "render/window.h"

#include <stdexcept>

namespace g4d {

window::window(int width, int height, const std::string& title) {
    if (!glfwInit()) throw std::runtime_error("glfwInit failed");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    handle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!handle) {
        glfwTerminate();
        throw std::runtime_error("could not create window / OpenGL 3.3 context");
    }
    glfwMakeContextCurrent(handle);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        glfwDestroyWindow(handle);
        glfwTerminate();
        throw std::runtime_error("gladLoadGLLoader failed");
    }

    glfwSwapInterval(1);
    glfwSetWindowUserPointer(handle, this);

    glfwSetKeyCallback(handle, [](GLFWwindow* h, int key, int, int action, int) {
        auto* self = static_cast<window*>(glfwGetWindowUserPointer(h));
        if (self->on_key) self->on_key(key, action);
    });
    glfwSetMouseButtonCallback(handle, [](GLFWwindow* h, int button, int action, int) {
        auto* self = static_cast<window*>(glfwGetWindowUserPointer(h));
        if (self->on_mouse_button) self->on_mouse_button(button, action);
    });
}

window::~window() {
    glfwDestroyWindow(handle);
    glfwTerminate();
}

bool window::should_close() const { return glfwWindowShouldClose(handle); }
void window::close() { glfwSetWindowShouldClose(handle, GLFW_TRUE); }
void window::poll_events() { glfwPollEvents(); }
void window::swap_buffers() { glfwSwapBuffers(handle); }
void window::set_title(const std::string& title) { glfwSetWindowTitle(handle, title.c_str()); }
void window::set_vsync(bool on) { glfwSwapInterval(on ? 1 : 0); }
double window::time() const { return glfwGetTime(); }

void window::framebuffer_size(int& w, int& h) const { glfwGetFramebufferSize(handle, &w, &h); }

void window::cursor_position(double& x, double& y) const {
    // GLFW reports the cursor in window (screen) coordinates; scale to framebuffer pixels
    int ww, wh, fw, fh;
    glfwGetWindowSize(handle, &ww, &wh);
    glfwGetFramebufferSize(handle, &fw, &fh);
    glfwGetCursorPos(handle, &x, &y);
    if (ww > 0 && wh > 0) {
        x *= (double)fw / ww;
        y *= (double)fh / wh;
    }
}

}  // namespace g4d