#pragma once

#include <memory>
#include <string>
#include <vector>

#include "raster/frame.h"
#include "render/renderer.h"
#include "scene/scene.h"
#include "render/window.h"

namespace g4d {

/**
 * @brief Owns the window, renderer and frame, and runs the main loop
 *        for a list of scenes.
 *
 * Keys handled here: ESC quits, TAB switches scene, F5 reloads the shader files.
 * everything else, plus mouse input, is forwarded to the active scene.
 */
class app {
public:
    /**
     * @param width       rasterizer resolution (also the initial window size)
     * @param height      rasterizer resolution
     * @param title       window title prefix
     * @param shader_dir  directory with fullscreen.vert and present.frag
     */
    app(int width, int height, const std::string& title, const std::string& shader_dir);

    /** @brief registers a scene. The first one added is active at start. */
    void add_scene(std::unique_ptr<scene> s);

    /** @brief runs until the window is closed. needs at least one scene. */
    void run();

private:
    void next_scene();
    bool read_mouse();   // updates mouse_x/mouse_y, returns false if the cursor is off the image

    // declaration order matters: the window (GL context) must outlive the renderer
    window win;
    renderer rend;
    frame fr;
    std::string title;
    std::vector<std::unique_ptr<scene>> scenes;
    size_t current = 0;
    int mouse_x = 0, mouse_y = 0;
};

}  // namespace g4d