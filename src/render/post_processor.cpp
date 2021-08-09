#include "post_processor.h"

cr::post_processor::post_processor()
{
    // Load shaders in
    // Load the shader into the string
    {
        auto shader_file_in_stream =
          std::ifstream(std::string(CRENDER_ASSET_PATH) + "shaders/post_process.comp");
        auto shader_string_stream = std::stringstream();
        shader_string_stream << shader_file_in_stream.rdbuf();
        const auto shader_source = shader_string_stream.str();

        // Create OpenGL shader
        auto       shader_handle = glCreateShader(GL_COMPUTE_SHADER);
        const auto shader_string = shader_source.c_str();
        glShaderSource(shader_handle, 1, &shader_string, nullptr);
        glCompileShader(shader_handle);

        auto success = int(0);
        auto log     = std::array<char, 512>();
        glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &success);

        if (!success)
        {
            glGetShaderInfoLog(shader_handle, 512, nullptr, log.data());
            cr::logger::error(
              "Compiling shader [{}], with error [{}]\n",
              "post process",
              log.data());
        }
        _gpu_handles.compute_shader = shader_handle;
    }

    {
        // Create OpenGL program
        auto program_handle = glCreateProgram();

        glAttachShader(program_handle, _gpu_handles.compute_shader);
        glLinkProgram(program_handle);

        auto success = int(0);
        auto log     = std::array<char, 512>();
        glGetProgramiv(program_handle, GL_LINK_STATUS, &success);

        // If it failed, show the error message
        if (!success)
        {
            glGetProgramInfoLog(program_handle, 512, nullptr, log.data());
            cr::logger::error(
              "Linking program [{}], with error [{}]\n",
              program_handle,
              log.data());
        }
        _gpu_handles.compute_program = program_handle;
    }
}

cr::image cr::post_processor::process(const cr::image &image) const noexcept
{
    auto gpu_src_img = GLuint();
    glGenTextures(1, &gpu_src_img);
    glBindTexture(GL_TEXTURE_2D, gpu_src_img);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, gpu_src_img);
    glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGBA32F,
      image.width(),
      image.height(),
      0,
      GL_RGBA,
      GL_FLOAT,
      image.data());

    auto gpu_target_img = GLuint();
    glGenTextures(1, &gpu_target_img);
    glBindTexture(GL_TEXTURE_2D, gpu_target_img);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, image.width(), image.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    glUseProgram(_gpu_handles.compute_program);

    glBindTexture(GL_TEXTURE_2D, gpu_target_img);
    glClearTexImage(gpu_target_img, 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindImageTexture(0, gpu_target_img, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gpu_src_img);

    glUniform2i(
      glGetUniformLocation(_gpu_handles.compute_program, "scene_size"),
      image.width(),
      image.height());

    glDispatchCompute(
      static_cast<int>(glm::ceil(image.width() / 8)),
      static_cast<int>(glm::ceil(image.height() / 8)),
      1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    auto processed_image = cr::image(image.width(), image.height());
    glBindTexture(GL_TEXTURE_2D, gpu_target_img);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, processed_image.data());

    glDeleteTextures(1, &gpu_target_img);
    glDeleteTextures(1, &gpu_src_img);

    return processed_image;
}
