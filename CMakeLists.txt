#########################
# PROJECT CONFIGURATION #
#########################
cmake_minimum_required(VERSION 3.19)
set(CMAKE_CXX_STANDARD 20)

project(crebon)

add_subdirectory(external)

find_package(embree 3.12.1)

find_package(OpenImageDenoise 1.4.3 REQUIRED)

##########
# TARGET #
##########
add_executable(crebon
        source/main.cpp
        source/render/cpu_renderer.cpp 
        source/render/sky.hpp 
        source/render/sky.cpp 
        source/scene/triangular_scene.cpp              
        source/scene/sphere_scene.cpp
        source/material/brdf.cpp
        source/ui/components/post_processing.cpp
        source/ui/components/render_target.cpp 
        source/ui/components/render_target.h 
        source/ui/components/asset_loader.cpp 
        source/ui/components/asset_loader.hpp 
        source/ui/components/component.hpp
        source/ui/components/settings.cpp
        source/ui/components/preview.cpp
        source/ui/components/console.cpp
        source/ui/components/export.cpp 
        source/ui/components/export.hpp 
        source/ui/components/skybox.cpp 
        source/ui/components/skybox.hpp 
        source/ui/components/stats.cpp
        source/ui/components/stats.hpp
        source/ui/input_devices.cpp
        source/ui/display.cpp
        source/util/logger.hpp
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
add_subdirectory(external/glfw)
add_subdirectory(external/glm)
add_subdirectory(external/fmt)

target_link_libraries(crebon PRIVATE 
glm fmt embree glfw OpenImageDenoise
)

target_include_directories(crebon PRIVATE 
source external external/imgui
)

target_compile_definitions(crebon PRIVATE 
IMGUI_HAS_DOCKING GLFW_INCLUDE_NONE IMGUI_IMPL_OPENGL_LOADER_GLAD FMT_CONSTEVAL=
)