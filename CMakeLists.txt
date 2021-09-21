cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)
set_property(GLOBAL PROPERTY ALLOW_DUPLICATE_CUSTOM_TARGETS 1)

project(CRender)

set(EMBREE_ISA_AVX256 ON)

option(CRENDER_USE_RELATIVE_ASSET_PATH "Enabled during development, for production builds disable" OFF)
option(CRENDER_USE_TRACY "Enabled use Tracy, for production builds disable" OFF)

if (CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC" AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "Using clang-cl options (MSVC interface)")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR- /arch:AVX /MT /W4 /EHa /EHs")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "Using Clang/GCC compiler options")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
    if ((CMAKE_BUILD_TYPE MATCHES Release) OR (CMAKE_BUILD_TYPE MATCHES RelWithDbgInfo))
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")
    endif ()
endif ()

find_package(embree 3.0 REQUIRED)
find_package(OpenImageDenoise 1.4.0 REQUIRED)

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
        src/render/entities/components.h
        src/render/entities/registry.cpp
        src/render/entities/registry.h
        src/render/draft/draft_renderer.cpp
        src/render/draft/draft_renderer.h
        src/render/timer.cpp src/render/timer.h
        src/util/logger.h
        src/util/logger.cpp
        src/util/numbers.h
        src/render/brdf.h
        src/util/denoise.h
        src/render/gpu_renderer.cpp
        src/render/gpu_renderer.h
        src/util/opengl.h
        src/render/bvh_node.h
        src/math/bbox.h

        src/tracy/TracyClient.cpp
        )

target_include_directories(CRender PRIVATE src)
target_include_directories(CRender PRIVATE external)

target_link_libraries(CRender glfw fmt glm embree OpenImageDenoise)

if (CRENDER_USE_RELATIVE_ASSET_PATH)
    message("Using relative asset path")
    set(CRENDER_ASSET_PATH_VAR "./assets/app/")
else()
    message("Using none-relative asset path")
    set(CRENDER_ASSET_PATH_VAR "${CMAKE_SOURCE_DIR}/assets/app/")
endif()

if (CRENDER_USE_TRACY)
    message("Using Tracy")
    set(CRENDER_TRACY_MACRO -DTRACY-ENABLE)
else()
    set(CRENDER_TRACY_MACRO "")
endif()

target_compile_definitions(CRender PUBLIC -DGLFW_INCLUDE_NONE -D__STDC_CONSTANT_MACROS -DIMGUI_IMPL_OPENGL_LOADER_GLAD ${CRENDER_TRACY_MACRO} -DCRENDER_ASSET_PATH="${CRENDER_ASSET_PATH_VAR}")
