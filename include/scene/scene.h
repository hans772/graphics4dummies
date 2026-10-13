#pragma once

#include <GLFW/glfw3.h>   // for GLFW_KEY_* / GLFW_MOUSE_BUTTON_* constants (built with GLFW_INCLUDE_NONE)

#include "raster/frame.h"

namespace g4d {

/**
 * @brief scene which will be used by the app to render n screen
 *
 * every frme the app clears the screen
 *
 * Pixel coordinates passed to the mouse callbacks use the rasterizer's convention:
 * origin bottom-left, y up, 0 <= x < width, 0 <= y < height.
 *
 * The app reserves ESC (quit), TAB (next scene) and F5 (reload shaders).
 */
class scene {
public:
    virtual ~scene() = default;

    /** @brief Name shown in the window title. */
    virtual const char* name() const = 0;

    /** @brief Advances the scene by dt seconds. Called once per frame before render(). */
    virtual void update(double /*dt*/) {}

    /** @brief Draws the scene into an already cleared frame. */
    virtual void render(frame& f) = 0;

    /** @param key GLFW_KEY_*  @param action GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT */
    virtual void on_key(int /*key*/, int /*action*/) {}

    /** @param button GLFW_MOUSE_BUTTON_*  @param action GLFW_PRESS or GLFW_RELEASE */
    virtual void on_mouse_button(int /*button*/, int /*action*/, int /*x*/, int /*y*/) {}

    /** @brief Called when the cursor moves over the image. */
    virtual void on_mouse_move(int /*x*/, int /*y*/) {}
};

}  // namespace g4d