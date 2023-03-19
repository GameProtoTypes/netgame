project(game)
cmake_minimum_required(VERSION 3.25)


include(FetchContent)
FetchContent_Declare(
  Cista
  GIT_REPOSITORY https://github.com/felixguendling/cista
  GIT_TAG        master
)

FetchContent_MakeAvailable(Cista)


add_subdirectory(game)
add_subdirectory(SLikeNet)
add_subdirectory(angelscriptsdk/angelscript/projects/cmake)
add_subdirectory(gcem)


set(SPRINTFS_FUNCTION_EXISTS true)
set(VSNPRINTFS_FUNCTION_EXISTS true)

add_library(imgui
    # Main Imgui files
    ./imgui/imgui.cpp ./imgui/imgui_draw.cpp
    ./imgui/imgui_tables.cpp ./imgui/imgui_widgets.cpp

    #implot
    ./imgui/implot.cpp ./imgui/implot_items.cpp ./imgui/implot_demo.cpp

    # SDL2+OpenGL-specific files
    ./imgui/backends/imgui_impl_sdl.cpp
    ./imgui/backends/imgui_impl_opengl3.cpp)


set(SDL2_INCLUDE_DIRS ./SDL2-2.0.22/include)

# Make SDL2 available to Imgui
target_include_directories(imgui PUBLIC ${SDL2_INCLUDE_DIRS})
target_include_directories(imgui PUBLIC ./GLFW)
target_include_directories(imgui PUBLIC ./glew-2.1.0/include)
# imgui/backends/ contains the SDL implementation
target_include_directories(imgui PUBLIC ./imgui/ ./imgui/backends/)
# Configure Imgui to use OpengGL through SDL2
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_CUSTOM=<glfw3native.h>  GL_GLEXT_PROTOTYPES=1)



