
#include "app/app.h"

#include <cstdio>
#include <stdexcept>

namespace g4d {

app::app(int width, int height, const std::string& t, const std::string& shader_dir)
    : win(width, height, t), rend(width, height, shader_dir), fr(width, height), title(t) {
    win.on_key = [this](int key, int action) {
        if (action == GLFW_PRESS) {
            if (key == GLFW_KEY_ESCAPE) { win.close(); return; }
            if (key == GLFW_KEY_TAB)    { next_scene(); return; }
            if (key == GLFW_KEY_F5)     { rend.reload_shaders(); return; }
        }
        if (!scenes.empty()) scenes[current]->on_key(key, action);
    };

    win.on_mouse_button = [this](int button, int action) {
        read_mouse();   // uses the last position on the image if the cursor is outside it
        if (!scenes.empty()) scenes[current]->on_mouse_button(button, action, mouse_x, mouse_y);
    };
}

void app::add_scene(std::unique_ptr<scene> s) {
    scenes.push_back(std::move(s));
}

void app::next_scene() {
    if (scenes.empty()) return;
    current = (current + 1) % scenes.size();
    std::fprintf(stderr, "scene: %s\n", scenes[current]->name());
}

bool app::read_mouse() {
    double cx, cy;
    win.cursor_position(cx, cy);
    int px, py;
    if (!rend.screen_to_pixel(cx, cy, px, py)) return false;
    mouse_x = px;
    mouse_y = py;
    return true;
}

void app::run() {
    if (scenes.empty()) throw std::runtime_error("app::run() called with no scenes");

    double last = win.time(), title_time = last, raster_ms = 0;
    int frames = 0;

    while (!win.should_close()) {
        win.poll_events();

        double now = win.time();
        double dt = now - last;
        last = now;

        int old_x = mouse_x, old_y = mouse_y;
        if (read_mouse() && (mouse_x != old_x || mouse_y != old_y))
            scenes[current]->on_mouse_move(mouse_x, mouse_y);

        scene& s = *scenes[current];
        s.update(dt);

        double r0 = win.time();
        fr.clear();
        s.render(fr);
        raster_ms += (win.time() - r0) * 1000.0;

        int fbw, fbh;
        win.framebuffer_size(fbw, fbh);
        rend.upload(fr.get_buffer());
        rend.present(fbw, fbh);
        win.swap_buffers();

        frames++;
        if (now - title_time >= 0.5) {
            char buf[200];
            std::snprintf(buf, sizeof buf, "%s | %s | %.0f fps | raster %.2f ms",
                          title.c_str(), s.name(), frames / (now - title_time), raster_ms / frames);
            win.set_title(buf);
            frames = 0;
            raster_ms = 0;
            title_time = now;
        }
    }
}

}  // namespace g4d