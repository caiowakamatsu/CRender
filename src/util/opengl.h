#pragma once

#include <optional>
#include <string>
#include <fstream>
#include <sstream>
#include <array>
#include <filesystem>

#include <util/logger.h>

#include <glad/glad.h>

namespace cr::opengl
{
    /// Creates an opengl shader
    /// \param path The shader location on disk
    /// \param shader_type The OpenGL shader type. VERTEX_SHADER, FRAGMENT_SHADER, COMPUTE_SHADER
    /// \return The shader handle, if there's no value then there was an error creating it
    [[nodiscard]] inline GLuint create_shader(const std::string &path, GLuint shader_type)
    {
        auto shader_stream = std::ifstream(path);
        auto shader_string_stream = std::stringstream();

        shader_string_stream << shader_stream.rdbuf();
        const auto shader_source = shader_string_stream.str();
        const auto shader_string = shader_source.c_str();

        const auto shader_handle = glCreateShader(shader_type);
        glShaderSource(shader_handle, 1, &shader_string, nullptr);
        glCompileShader(shader_handle);

        auto success = int(0);
        auto log = std::array<char, 512>();
        glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &success);

        if (!success)
        {
            glGetShaderInfoLog(shader_handle, 512, nullptr, log.data());
            cr::logger::error("Compiling shader [{}], with error [{}]\n", path, log.data());
            return {};
        }
        return shader_handle;
    }

    /// Creates a program based on shaders pased in
    /// \tparam ...
    /// \param shaders The shaders you want to attach to the program being created
    /// \return The program handle, nullopt if it failed
    template <typename ...Shaders>
    [[nodiscard]] inline GLuint create_program(Shaders&&... shaders)
    {
        const auto handle = glCreateProgram();

        for (const auto shader : {shaders...})
            glAttachShader(handle, shader);

        glLinkProgram(handle);

        auto success = int(0);
        auto log = std::array<char, 512>();
        glGetProgramiv(handle, GL_LINK_STATUS, &success);

        if (!success)
        {
            glGetShaderInfoLog(handle, 512, nullptr, log.data());
            cr::logger::error("Linking program [{}], with error [{}]\n", handle, log.data());
            return {};
        }

        return handle;
    }
}
