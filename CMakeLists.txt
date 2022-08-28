#########################
# PROJECT CONFIGURATION #
#########################
cmake_minimum_required(VERSION 3.19)
set(CMAKE_CXX_STANDARD 20)

project(crebon)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-volatile")

add_subdirectory(external)

find_package(embree 3.12.1)

find_package(OpenImageDenoise 1.4.3 REQUIRED)

##########
# TARGET #
##########
add_executable(crebon
        source/main.cpp
        source/render/cpu_renderer.cpp 
        source/render/sky.cpp 
        source/scene/triangular_scene.cpp              
        source/scene/sphere_scene.cpp
        source/material/brdf.cpp
        source/ui/components/post_processing.cpp
        source/ui/components/render_target.cpp 
        source/ui/components/asset_loader.cpp 
        source/ui/components/settings.cpp
        source/ui/components/preview.cpp
        source/ui/components/console.cpp
        source/ui/components/export.cpp 
        source/ui/components/skybox.cpp 
        source/ui/components/stats.cpp
        source/ui/input_devices.cpp
        source/ui/display.cpp
        source/util/glad.c
        external/imgui/backends/imgui_impl_opengl3.cpp
        external/imgui/backends/imgui_impl_glfw.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui.cpp
)

#############
# LIBRARIES #
#############aarch64

target_link_libraries(crebon PRIVATE 
glm fmt embree glfw OpenImageDenoise
)

target_include_directories(crebon PRIVATE 
source external external/imgui
)

target_compile_definitions(crebon PRIVATE 
IMGUI_HAS_DOCKING GLFW_INCLUDE_NONE IMGUI_IMPL_OPENGL_LOADER_GLAD FMT_CONSTEVAL=
)