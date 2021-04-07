cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)
set_property(GLOBAL PROPERTY ALLOW_DUPLICATE_CUSTOM_TARGETS 1)

project(CRender)

set(EMBREE_ISA_AVX512 OFF)
set(EMBREE_ISA_AVX2 NONE)

set(embree_DIR "C:\\Program Files\\Intel\\Embree3\\lib\\cmake\\embree-3.12.2")
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
        src/objects/image.cpp
        src/objects/image.h
        src/codecs/video_reader.cpp
        src/codecs/video_reader.h
        src/util/colour.h
        src/codecs/video_writer.cpp
        src/codecs/video_writer.h
        src/render/ray.cpp
        src/render/ray.h
        src/render/camera.cpp
        src/render/camera.h
        src/objects/bit.h
        src/render/scene.cpp
        src/render/scene.h
        src/util/algorithm.h src/render/material/material.cpp src/render/material/material.h src/util/model_loader.cpp src/util/model_loader.h src/objects/thread_pool.cpp src/objects/thread_pool.h src/ui/nodes/node_editor.cpp src/ui/nodes/node_editor.h src/util/sampling.h src/render/light.cpp src/render/light.h src/objects/model.cpp src/objects/model.h)

target_include_directories(CRender PRIVATE src)
target_include_directories(CRender PRIVATE external)

if (WIN32)
    target_include_directories(CRender PRIVATE win-lib/include)

    file(GLOB ffmpeg_sources win-lib/bin)

    target_link_libraries(CRender
            glfw
            fmt
            glm
            avdevice
            avfilter
            avformat
            avcodec
            swresample
            swscale
            avutil
            embree
            )
    target_link_directories(CRender PUBLIC ${CMAKE_SOURCE_DIR}/win-lib/bin)
endif ()

if (UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
            libavdevice
            libavfilter
            libavformat
            libavcodec
            libswresample
            libswscale
            libavutil
            )

    target_link_libraries(CRender glfw fmt glm PkgConfig::LIBAV embree)
endif ()

target_compile_definitions(CRender PUBLIC -DGLFW_INCLUDE_NONE -D__STDC_CONSTANT_MACROS)
