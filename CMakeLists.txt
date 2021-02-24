cmake_minimum_required(VERSION 3.19)
project(CRender)

add_subdirectory(external)

set(CMAKE_CXX_STANDARD 17)

FILE(GLOB ImGuiSources src/imgui/*)

add_executable(CRender src/main.cpp src/glad/glad.h src/glad/glad.c ${ImGuiSources} src/imgui/imnodes.h src/imgui/imnodes.cpp src/ui/display.cpp src/ui/display.h src/ui/user_settings.h src/ui/themes.h src/util/exception.h src/asset/texture.cpp src/asset/texture.h)

target_include_directories(CRender PRIVATE src)
target_include_directories(CRender PRIVATE external)

#target_compile_definitions(CRender PUBLIC -DGLFW_INCLUDE_NONE)

target_link_libraries(CRender glfw fmt)
