cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)
set_property(GLOBAL PROPERTY ALLOW_DUPLICATE_CUSTOM_TARGETS 1)

project(CRender)

set(EMBREE_ISA_AVX256 ON)

if (CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC" AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    message(STATUS "Using clang-cl options (MSVC interface)")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR- /arch:AVX /MT /W4 /EHa /EHs")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "Using Clang/GCC compiler options")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native -fpermissive")
    if ((CMAKE_BUILD_TYPE MATCHES Release) OR (CMAKE_BUILD_TYPE MATCHES RelWithDbgInfo))
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")
    endif ()
endif ()

find_package(embree 3.0 REQUIRED)
find_package(OpenImageDenoise 1.4.0 REQUIRED)
# find_package(OpenGL REQUIRED)

add_subdirectory(external)

FILE(GLOB ImGuiSources src/imgui/*)

#I don't bloody know why mesh.h & entity_type.h is missing
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
#        src/render/entities/entity_type.h
        src/render/entities/components.h
        src/render/entities/registry.cpp
        src/render/entities/registry.h 
        src/render/draft/draft_renderer.cpp 
        src/render/draft/draft_renderer.h 
#        src/render/mesh.h 
        src/render/timer.cpp 
        src/render/timer.h 
        src/util/logger.h 
        src/util/logger.cpp 
        src/util/numbers.h 
        src/render/brdf.h 
        src/util/denoise.h)

target_include_directories(CRender PRIVATE src)
target_include_directories(CRender PRIVATE external)

#Wasn't this broken like yesterday????? Maybe it was because of the setvars stuff
target_link_libraries(CRender glfw fmt glm embree OpenImageDenoise stdc++fs)

target_compile_definitions(CRender PUBLIC -DGLFW_INCLUDE_NONE -D__STDC_CONSTANT_MACROS -DIMGUI_IMPL_OPENGL_LOADER_GLAD)
