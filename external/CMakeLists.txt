option(EMBREE_ISPC_SUPPORT "" OFF)

option(GLFW_BUILD_TESTS "" OFF)
option(GLFW_BUILD_DOCS "" OFF)
option(GLFW_INSTALL "" OFF)
option(GLFW_BUILD_EXAMPLES "" OFF)

option(FMT_PEDANTIC  "" OFF)
option(FMT_WERROR   "" OFF)
option(FMT_DOC   "" OFF)
option(FMT_TEST   "" OFF)
option(FMT_FUZZ   "" OFF)
option(FMT_CUDA_TEST   "" OFF)

include(FetchContent)

FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw
        GIT_TAG 3.3.2
)

FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt
        GIT_TAG 7.1.3
)

FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm
        GIT_TAG 0.9.9.8
)

FetchContent_Declare(
        bvh
        GIT_REPOSITORY https://github.com/madmann91/bvh
        GIT_TAG bc3eff9f961ab1d9dd39a3fc9c43ace59a2ea35c
)

FetchContent_MakeAvailable(glfw fmt glm bvh)
