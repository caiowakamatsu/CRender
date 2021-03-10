cmake_minimum_required(VERSION 3.16)
project(CRender)

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

add_subdirectory(external)

set(CMAKE_CXX_STANDARD 17)

FILE(GLOB ImGuiSources src/imgui/*)

add_executable(CRender src/main.cpp src/glad/glad.h src/glad/glad.c ${ImGuiSources} src/imgui/imnodes.h src/imgui/imnodes.cpp src/ui/display.cpp src/ui/display.h src/ui/user_settings.h src/ui/themes.h src/util/exception.h src/render/renderer.cpp src/render/renderer.h src/render/bvh_node.cpp src/render/bvh_node.h src/objects/image.cpp src/objects/image.h src/codecs/video_reader.cpp src/codecs/video_reader.h src/util/colour.h src/codecs/video_writer.cpp src/codecs/video_writer.h src/world/world.cpp src/world/world.h)

target_include_directories(CRender PRIVATE src)
target_include_directories(CRender PRIVATE external)

target_compile_definitions(CRender PUBLIC -DGLFW_INCLUDE_NONE -D__STDC_CONSTANT_MACROS)

target_link_libraries(CRender glfw fmt glm PkgConfig::LIBAV)
