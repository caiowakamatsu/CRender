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

    {
        auto shader_file_in_stream =
          std::ifstream(std::string(CRENDER_ASSET_PATH) + "shaders/blur.comp");
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
        _gpu_handles.blur_shader = shader_handle;
    }

    {
        // Create OpenGL program
        auto program_handle = glCreateProgram();

        glAttachShader(program_handle, _gpu_handles.blur_shader);
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
              "Blur Program",
              log.data());
        }
        _gpu_handles.blur_program = program_handle;
    }
}

cr::image cr::post_processor::process(const cr::image &image) const noexcept
{
    if (
      !_bloom_settings.enabled && !_gray_scale_settings.enabled && !_tonemapping_settings.enabled)
        return image;    // Short circuit if there's no post being done

    auto bloom_img = GLuint();
    if (_bloom_settings.enabled)
    {
        const auto blurred = _blur(_brightness(image), glm::ivec2(image.width(), image.height()));

        glGenTextures(1, &bloom_img);
        glBindTexture(GL_TEXTURE_2D, bloom_img);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(
          GL_TEXTURE_2D,
          0,
          GL_RGBA32F,
          image.width(),
          image.height(),
          0,
          GL_RGBA,
          GL_FLOAT,
          blurred.data());
    }
    glUseProgram(_gpu_handles.compute_program);

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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGBA32F,
      image.width(),
      image.height(),
      0,
      GL_RGB,
      GL_UNSIGNED_BYTE,
      nullptr);

    glBindTexture(GL_TEXTURE_2D, gpu_target_img);
    glClearTexImage(gpu_target_img, 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindImageTexture(0, gpu_target_img, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gpu_src_img);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bloom_img);

    glUniform2i(
      glGetUniformLocation(_gpu_handles.compute_program, "scene_size"),
      image.width(),
      image.height());

    glUniform1i(
      glGetUniformLocation(_gpu_handles.compute_program, "use_bloom"),
      _bloom_settings.enabled);

    glUniform1i(
      glGetUniformLocation(_gpu_handles.compute_program, "use_gray_scale"),
      _gray_scale_settings.enabled);

    glUniform1i(
      glGetUniformLocation(_gpu_handles.compute_program, "use_tonemapping"),
      _tonemapping_settings.enabled);

    glUniform1f(
      glGetUniformLocation(_gpu_handles.compute_program, "bloom_strength"),
      _bloom_settings.strength);

    glUniform1i(
      glGetUniformLocation(_gpu_handles.compute_program, "tonemapping_type"),
      _tonemapping_settings.type);

    glUniform1f(
      glGetUniformLocation(_gpu_handles.compute_program, "tonemapping_exposure"),
      _tonemapping_settings.exposure);

    glUniform1f(
      glGetUniformLocation(_gpu_handles.compute_program, "gamma_correction"),
      _tonemapping_settings.gamma_correction);

    glDispatchCompute(
      static_cast<int>(glm::ceil(image.width() / 8)),
      static_cast<int>(glm::ceil(image.height() / 8)),
      1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

    auto processed_image = cr::image(image.width(), image.height());
    glBindTexture(GL_TEXTURE_2D, gpu_target_img);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, processed_image.data());

    glDeleteTextures(1, &gpu_target_img);
    glDeleteTextures(1, &gpu_src_img);

    return processed_image;
}

cr::image cr::post_processor::_blur(const cr::image &source_img, const glm::ivec2 &dimensions)
  const noexcept
{
    auto buffers = std::array<GLuint, 2>();    // ping pong between the buffers
    glUseProgram(_gpu_handles.blur_program);
    glGenTextures(2, buffers.data());
    for (auto i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, buffers[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, buffers[i]);
        glTexImage2D(
          GL_TEXTURE_2D,
          0,
          GL_RGBA32F,
          dimensions.x,
          dimensions.y,
          0,
          GL_RGBA,
          GL_FLOAT,
          i == 0 ? source_img.data() : nullptr);
    }

    for (auto i = 0; i < 10; i++)
    {
        const auto source = i % 2 == 0 ? buffers[0] : buffers[1];
        const auto target = i % 2 != 0 ? buffers[0] : buffers[1];

        glBindTexture(GL_TEXTURE_2D, target);
        glClearTexImage(target, 0, GL_RGBA, GL_FLOAT, nullptr);
        glBindImageTexture(0, target, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, source);

        glUniform2i(
          glGetUniformLocation(_gpu_handles.blur_program, "scene_size"),
          dimensions.x,
          dimensions.y);

        glUniform1i(glGetUniformLocation(_gpu_handles.blur_program, "horizontal"), i % 2 == 0);

        glDispatchCompute(
          static_cast<int>(glm::ceil(dimensions.x / 8)),
          static_cast<int>(glm::ceil(dimensions.y / 8)),
          1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
    }

    auto blurred = cr::image(dimensions.x, dimensions.y);
    glBindTexture(GL_TEXTURE_2D, buffers[0]);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, blurred.data());
    glDeleteTextures(2, buffers.data());

    return blurred;
}

cr::image cr::post_processor::_brightness(const cr::image &source) const noexcept
{
    auto passed = source;

    for (auto i = 0; i < passed.height() * passed.width(); i++)
    {
        const auto at         = glm::vec3(passed.get(i % source.width(), i / source.height()));
        const auto brightness = glm::dot(at, glm::vec3(0.2126, 0.7152, 0.0722));
        if (brightness < _bloom_settings.threshold)
            passed.set(i % source.width(), i / source.height(), glm::vec3(0));
    }

    return passed;
}

void cr::post_processor::submit_bloom_settings(const cr::post_processor::bloom_settings &settings)
{
    _bloom_settings = settings;
}

void cr::post_processor::submit_gray_scale_settings(
  const cr::post_processor::gray_scale_settings &settings)
{
    _gray_scale_settings = settings;
}

void cr::post_processor::submit_tonemapping_settings(
  const cr::post_processor::tonemapping_settings &settings)
{
    _tonemapping_settings = settings;
}
