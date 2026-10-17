cmake_minimum_required(VERSION 3.20)
project(
  graphics4dummies
  VERSION 0.1
  LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # for clangd / IDEs

# Default to Release: an unoptimized rasterizer is 10-50x slower and will ruin
# your timings
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE
      Release
      CACHE STRING "" FORCE)
endif()

option(BUILD_VIEWER "Build the OpenGL viewer" OFF)
option(BUILD_EXAMPLES "Build example programs" OFF)
option(BUILD_BENCH "Build benchmarks" OFF)
option(BUILD_TESTS "Build tests" OFF)
option(BUILD_ALL "Build all" OFF)


include(FetchContent)

if(BUILD_ALL)
  set(GLFW_BUILD_DOCS
      OFF
      CACHE BOOL "" FORCE)
  set(GLFW_BUILD_TESTS
      OFF
      CACHE BOOL "" FORCE)
  set(GLFW_BUILD_EXAMPLES
      OFF
      CACHE BOOL "" FORCE)
  set(GLFW_INSTALL
      OFF
      CACHE BOOL "" FORCE)
  FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
    )
  FetchContent_MakeAvailable(glfw)
endif()

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.1
  GIT_PROGRESS TRUE
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(glm)

# ---------- warnings helper ----------
add_library(project_warnings INTERFACE)
if(MSVC)
  target_compile_options(project_warnings INTERFACE /W4)
else()
  target_compile_options(project_warnings INTERFACE -Wall -Wextra -Wpedantic)
endif()

file(GLOB_RECURSE RASTER_SRC CONFIGURE_DEPENDS src/raster/*.cpp)

add_library(raster STATIC ${RASTER_SRC})
target_include_directories(raster PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(
  raster
  PUBLIC glm::glm
  PRIVATE project_warnings)

# header-only image writer for PNG export
add_library(stb INTERFACE)
target_include_directories(stb SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/stb)

# ---------- viewer ----------
if(BUILD_VIEWER OR BUILD_ALL)
  add_library(glad STATIC third_party/glad/src/glad.c)
  target_include_directories(glad PUBLIC third_party/glad/include)
  target_link_libraries(glad PUBLIC ${CMAKE_DL_LIBS})

  file(GLOB_RECURSE VIEWER_SRC CONFIGURE_DEPENDS
    src/render/*.cpp src/app/*.cpp src/scenes/*.cpp src/util/*.cpp)
  add_library(viewer_src STATIC ${VIEWER_SRC})
  target_include_directories(viewer_src PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
    )
  target_compile_definitions(viewer_src PUBLIC GLFW_INCLUDE_NONE)
  target_link_libraries(viewer_src
    PUBLIC  raster stb glm::glm glfw glad
    PRIVATE project_warnings)

  file(GLOB VIEWER_SRCS CONFIGURE_DEPENDS src/*.cpp)
  foreach(src ${VIEWER_SRCS})
    get_filename_component(name ${src} NAME_WE)
    add_executable(${name}_viewer ${src})
    target_compile_definitions(${name}_viewer PRIVATE
      G4D_SHADER_DIR="${CMAKE_SOURCE_DIR}/assets/shaders"
      G4D_FONTS_DIR="$(CMAKE_SOURCE_DIR)/assets/fonts"
    )
    target_link_libraries(${name}_viewer PRIVATE raster stb viewer_src project_warnings)
  endforeach()

endif()

if(BUILD_EXAMPLES OR BUILD_ALL)
  file(GLOB EXAMPLE_SRCS CONFIGURE_DEPENDS examples/*.cpp)
  foreach(src ${EXAMPLE_SRCS})
    get_filename_component(name ${src} NAME_WE)
    add_executable(${name} ${src})
    target_link_libraries(${name} PRIVATE raster stb project_warnings)
  endforeach()
endif()

if(BUILD_BENCH OR BUILD_ALL)
  file(GLOB BENCH_SRCS CONFIGURE_DEPENDS bench/*.cpp)
  foreach(src ${BENCH_SRCS})
    get_filename_component(name ${src} NAME_WE)
    add_executable(bench_${name} ${src})
    target_link_libraries(bench_${name} PRIVATE raster stb project_warnings)
  endforeach()
endif()

if(BUILD_TESTS OR BUILD_ALL)
  enable_testing()
  add_subdirectory(tests)
endif()

find_package(Doxygen QUIET COMPONENTS doxygen dot)
if(DOXYGEN_FOUND)
  set(DOXYGEN_GENERATE_HTML YES)
  set(DOXYGEN_EXTRACT_ALL YES)
  set(DOXYGEN_RECURSIVE YES)
  
  set(DOXYGEN_FILE_PATTERNS "*.cpp" "*.h" "*.hpp" "*.c" "*.md")
  set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/docs/)
  set(DOXYGEN_INPUT 
    ${CMAKE_CURRENT_SOURCE_DIR}/src 
    ${CMAKE_CURRENT_SOURCE_DIR}/include 
    ${CMAKE_CURRENT_SOURCE_DIR}/examples 
    ${CMAKE_CURRENT_SOURCE_DIR}/README.md
    )
  
  set(DOXYGEN_USE_MDFILE_AS_MAINPAGE ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
  
  set(DOXYGEN_HAVE_DOT YES) 
  set(DOXYGEN_CALL_GRAPH YES)
  
  doxygen_add_docs(docs 
    ${CMAKE_CURRENT_SOURCE_DIR}/src 
    ${CMAKE_CURRENT_SOURCE_DIR}/include 
    ${CMAKE_CURRENT_SOURCE_DIR}/examples 
    ${CMAKE_CURRENT_SOURCE_DIR}/README.md 
    COMMENT "Generating API docs"
  )
endif()
