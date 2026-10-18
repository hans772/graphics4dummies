# graphics4dummies

`graphics4dummies` is a small educational C++20 graphics project. It contains a
CPU rasterizer for drawing pixels, lines, circles, ellipses, text, and tree
structures, plus an optional OpenGL viewer that displays the rasterized frame
buffer in a GLFW window.

The project is intended for experimenting with fundamental computer graphics
algorithms rather than for production rendering. The rasterizer does not use
OpenGL to draw primitives: it writes pixels into a CPU-side buffer, and the
viewer only uploads that buffer and presents it on screen.

## Features

- Pixel, line, circle, and ellipse drawing.
- Unclipped line drawing with `frame::draw_line`.
- Cohen–Sutherland line clipping with `frame::draw_line_coh_sut`.
- Cyrus–Beck line clipping with `frame::draw_line_cyr_bec`.
- A reusable `frame` class storing a 32-bit pixel buffer.
- Bitmap-font text rendering through the bundled `kitchen_sink` font.
- Generic rasterized binary trees and AVL trees with labels and balance-factor
  annotations.
- Optional GLFW/OpenGL 3.3 viewer with shader hot reload.
- PNG-writing examples using the bundled `stb_image_write` header.
- Doxygen API documentation when Doxygen is installed.

## Repository layout

```text
include/
  app/       Application loop and scene management
  raster/    CPU rasterizer, text rendering, and tree data structures
  render/    GLFW window and OpenGL presentation layer
  scene/     Interactive viewer scenes
  util/      Fonts, colors, shapes, and tree helpers
src/
  app/       Application implementation
  raster/    Rasterizer implementation
  render/    Window and renderer implementation
  scenes/    Interactive scene implementations
  util/      Font implementation
  *.cpp      Viewer entry points
examples/    Small programs that write rasterized images to PNG files
bench/       Benchmark programs
assets/
  shaders/   Fullscreen presentation shaders
  fonts/     Bitmap font assets
third_party/ Vendored glad and stb headers
docs/        Generated API documentation, when generated
```

## Requirements

- CMake 3.20 or newer.
- A compiler with C++20 support:
  - GCC 10 or newer, or
  - Clang 12 or newer, or
  - Visual Studio 2019 16.10 or newer.
- Git and network access during the first CMake configure. CMake downloads
  GLM 1.0.1, and the viewer configuration also downloads GLFW 3.4.
- For the viewer: an OpenGL 3.3-capable system and the platform's OpenGL
  development libraries.
- For Doxygen documentation: Doxygen is optional. Graphviz is recommended for
  call graphs.

The bundled `glad` and `stb` sources do not need to be installed separately.

## Building

The default CMake build type is `Release`, which is recommended because the
CPU rasterizer is intentionally simple and can be substantially slower in a
debug build.

### Build the rasterizer library

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

This builds the static `raster` library. GLM is made available automatically
by CMake's `FetchContent`.

### Build the PNG examples

```sh
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_EXAMPLES=ON
cmake --build build --parallel
```

The resulting executables are:

- `draw_line`
- `draw_circle`
- `draw_ellipse`
- `cohen_sutherland`
- `cyrus_beck`

### Build the interactive viewers

```sh
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_VIEWER=ON
cmake --build build --parallel
```

This produces:

- `demo_viewer`: an interactive line, circle, and ellipse playground.
- `tree_viewer`: an interactive ordinary binary search tree.
- `bin_tree_viewer`: an interactive AVL tree.

The viewer build downloads GLFW 3.4 through CMake. On Linux, the GLFW build
may also require the development packages provided by the operating system for
X11 or Wayland, OpenGL, and related input libraries.

### Build examples, viewers, and benchmarks together

```sh
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_EXAMPLES=ON \
  -DBUILD_VIEWER=ON \
  -DBUILD_BENCH=ON
cmake --build build --parallel
```

The `BUILD_TESTS` option is present in the CMake configuration, but this
repository does not currently contain test sources.

## Running the examples

Run examples from the directory where the output PNG should be created:

```sh
mkdir -p output
cd output
../build/draw_line
../build/draw_circle
../build/draw_ellipse
../build/cohen_sutherland
../build/cyrus_beck
```

Each example writes one PNG file to the current working directory. The
examples are intentionally short and are useful starting points for adding
new drawing algorithms.

## Running the viewers

From the repository root:

```sh
./build/demo_viewer
./build/tree_viewer
./build/bin_tree_viewer
```

The programs use the repository's `assets/shaders` and `assets/fonts`
directories by default. A custom shader directory can be supplied as the
first argument:

```sh
./build/demo_viewer /path/to/shaders
```

The custom directory must contain the presentation shaders expected by the
renderer: `screen.vert` and `screen.frag`.

### Demo viewer controls

The demo starts in line-drawing mode.

| Input | Action |
| --- | --- |
| `1` | Select circle drawing |
| `2` | Select ellipse drawing |
| `3` | Select line drawing |
| Mouse press and release | Place the selected shape |
| `F5` | Reload the presentation shaders |
| `Esc` | Quit |

The first mouse click anchors a shape, the cursor previews its size, and the
button release commits it to the scene.

### Tree viewer controls

Both tree viewers accept integer values from the keyboard.

| Input | Action |
| --- | --- |
| Number keys and `-` | Enter an integer |
| `Backspace` | Remove the last entered character |
| `Enter` | Insert or delete the entered value |
| `Tab` | Toggle insert mode and delete mode |
| `F5` | Reload the presentation shaders |
| `Esc` | Quit |

Insert mode is shown with a green prompt. Delete mode is shown with a red
prompt. `tree_viewer` uses `g4d::raster_tree`, while `bin_tree_viewer` uses
`g4d::avl_tree`.

## Using the rasterizer in a program

The main public type is `g4d::frame`. A frame owns a `width` by `height`
buffer, and drawing operations write directly into that buffer.

```cpp
#include "raster/frame.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main() {
    g4d::frame image(320, 240);
    image.clear();
    image.draw_line(10, 10, 300, 200, 0xffffffff);
    image.draw_circle(160, 120, 60, 0xffffffff);

    stbi_write_png(
        "image.png",
        static_cast<int>(image.width()),
        static_cast<int>(image.height()),
        4,
        image.get_buffer(),
        static_cast<int>(image.width() * sizeof(uint32_t)));
}
```

Add the repository's `include` directory and link against the `raster` target
when integrating the library into another CMake project. The PNG writer is
optional; include `third_party/stb/stb_image_write.h` only in a translation
unit that defines `STB_IMAGE_WRITE_IMPLEMENTATION`.

Colors are passed as 32-bit values documented by the library as
`0xAARRGGBB`. Drawing methods safely skip pixels outside the frame. The
ordinary line, circle, and ellipse methods still iterate over their complete
geometry, whereas the two clipped line methods reject or trim portions
outside the frame using their respective algorithms.

## CMake options

All options default to `OFF` unless noted otherwise.

| Option | Description |
| --- | --- |
| `BUILD_VIEWER` | Build the GLFW/OpenGL viewer library and viewer executables |
| `BUILD_EXAMPLES` | Build the PNG example programs |
| `BUILD_BENCH` | Build programs from `bench/` with a `bench_` prefix |
| `BUILD_TESTS` | Enable the test configuration when test sources are available |
| `BUILD_ALL` | Enable all project components; use the individual options when selecting a specific subset |

Useful CMake variables include `CMAKE_BUILD_TYPE` and
`CMAKE_EXPORT_COMPILE_COMMANDS`. The latter is enabled by the project by
default and generates `build/compile_commands.json` for tools such as
clangd.

## Documentation

If Doxygen is installed, configure and build the `docs` target:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target docs
```

HTML documentation is written to `docs/api/html/`. The generated documentation
uses this README as its main page and includes the public headers, source
files, and examples.

## Design notes

- Rasterization and presentation are separate. `g4d::frame` is usable without
  GLFW or an OpenGL context.
- `g4d::renderer` does not rasterize primitives; it uploads a completed frame
  buffer to one OpenGL texture and presents it with a fullscreen triangle.
- The viewer keeps a fixed rasterizer resolution while fitting the image to
  the window and preserving its aspect ratio.
- `g4d::scene` provides the small interface used by the interactive
  application loop. New scenes can be registered with `g4d::app`.
- Tree layout and drawing are implemented in reusable templates, allowing
  ordinary and AVL trees to share the same scene.

## Project status

This is an educational and actively evolving project. APIs and source layout
may change as additional rasterization algorithms and demonstrations are
added. There is currently no published release or package manager
distribution.

## Credits & References

C. Wetherell and A. Shannon, “Tidy Drawing of Trees,” IEEE Trans. on Software
Engineering, vol. SE-5, no. 5, pp. 514-520, 1979.