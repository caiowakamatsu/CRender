cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)
set_property(GLOBAL PROPERTY ALLOW_DUPLICATE_CUSTOM_TARGETS 1)

project(CRender)

set(EMBREE_ISA_AVX512 OFF)
set(EMBREE_ISA_AVX2 NONE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR- /arch:AVX /MT /W4 /EHa /EHs")

find_package(embree 3.0 REQUIRED)

add_subdirectory(external)

FILE(GLOB ImGuiSources src/imgui/*)

add_executable(CRender src/main.cpp
        src/glad/glad.h
        src/glad/glad.c
        ${ImGuiSources}
        src/imgui/imnodes.h
        src/imgui/imnodes.cpp
        src/ui/display.cpp
        src/ui/display.h
        src/ui/user_settings.h
        src/ui/themes.h
        src/util/exception.h
        src/render/renderer.cpp
        src/render/renderer.h
        src/objects/image.h
        src/util/colour.h
        src/render/ray.cpp
        src/render/ray.h
        src/render/camera.cpp
        src/render/camera.h
        src/objects/bit.h
        src/render/scene.cpp
        src/render/scene.h
        src/util/algorithm.h
        src/render/material/material.cpp
        src/render/material/material.h
        src/util/asset_loader.cpp
        src/util/asset_loader.h
        src/objects/thread_pool.cpp
        src/objects/thread_pool.h
        src/ui/nodes/node_editor.cpp
        src/ui/nodes/node_editor.h
        src/util/sampling.h
        src/objects/model.cpp
        src/objects/model.h
        src/ui/ui.h
        src/render/entities/entity_type.h
        src/render/entities/components.h
        src/render/entities/registry.cpp
        src/render/entities/registry.h src/render/draft/draft_renderer.cpp src/render/draft/draft_renderer.h src/render/mesh.h src/render/timer.cpp src/render/timer.h src/util/logger.h src/util/logger.cpp)

target_include_directories(CRender PRIVATE src)
target_include_directories(CRender PRIVATE external)

target_link_libraries(CRender glfw fmt glm embree)

target_compile_definitions(CRender PUBLIC -DGLFW_INCLUDE_NONE -D__STDC_CONSTANT_MACROS)
