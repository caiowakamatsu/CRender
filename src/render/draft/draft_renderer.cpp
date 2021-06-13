#include "draft_renderer.h"

cr::draft_renderer::draft_renderer(
  uint64_t                    res_x,
  uint64_t                    res_y,
  std::unique_ptr<cr::scene> *scene)
    : _res_x(res_x), _res_y(res_y), _scene(scene)
{
    glGenFramebuffers(1, &_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);

    glGenTextures(1, &_texture);

    glBindTexture(GL_TEXTURE_2D, _texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _res_x, _res_y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture, 0);

    glGenRenderbuffers(1, &_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, _rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, _res_x, _res_y);

    glFramebufferRenderbuffer(
      GL_FRAMEBUFFER,
      GL_DEPTH_STENCIL_ATTACHMENT,
      GL_RENDERBUFFER,
      _rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        fmt::print("[ERROR] Framebuffer is not complete");

    glBindRenderbuffer(GL_FRAMEBUFFER, 0);

    // Load shaders in
    // Load the shader into the string
    {
        auto shader_file_in_stream = std::ifstream("./assets/shaders/shader.vert");
        auto shader_string_stream  = std::stringstream();
        shader_string_stream << shader_file_in_stream.rdbuf();
        const auto shader_source = shader_string_stream.str();

        // Create OpenGL shader
        auto       shader_handle = glCreateShader(GL_VERTEX_SHADER);
        const auto shader_string = shader_source.c_str();
        glShaderSource(shader_handle, 1, &shader_string, nullptr);
        glCompileShader(shader_handle);

        auto success = int(0);
        auto log     = std::array<char, 512>();
        glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &success);

        if (!success)
        {
            glGetShaderInfoLog(shader_handle, 512, nullptr, log.data());
            fmt::print("[ERROR] Compiling shader [{}], with error [{}]\n", "vertex", log.data());
        }
        _vertex_handle = shader_handle;
    }

    {
        auto shader_file_in_stream = std::ifstream("./assets/shaders/shader.frag");
        auto shader_string_stream  = std::stringstream();
        shader_string_stream << shader_file_in_stream.rdbuf();
        const auto shader_source = shader_string_stream.str();

        // Create OpenGL shader type
        auto       shader_handle = glCreateShader(GL_FRAGMENT_SHADER);
        const auto shader_string = shader_source.c_str();
        glShaderSource(shader_handle, 1, &shader_string, nullptr);
        glCompileShader(shader_handle);

        // Check if the shader compiled
        auto success = int(0);
        auto log     = std::array<char, 512>();
        glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &success);

        // If it failed, show the error message
        if (!success)
        {
            glGetShaderInfoLog(shader_handle, 512, nullptr, log.data());
            fmt::print("[ERROR] Compiling shader [{}], with error [{}]\n", "frag", log.data());
        }
        _fragment_handle = shader_handle;
    }

    {
        // Create OpenGL program
        auto program_handle = glCreateProgram();

        glAttachShader(program_handle, _vertex_handle);
        glAttachShader(program_handle, _fragment_handle);
        glLinkProgram(program_handle);

        auto success = int(0);
        auto log     = std::array<char, 512>();
        glGetProgramiv(program_handle, GL_LINK_STATUS, &success);

        // If it failed, show the error message
        if (!success)
        {
            glGetProgramInfoLog(program_handle, 512, nullptr, log.data());
            fmt::print(
              "[ERROR] Linking program [{}], with error [{}]\n",
              program_handle,
              log.data());
        }
        _program_handle = program_handle;
    }
}

GLuint cr::draft_renderer::rendered_texture() const
{
    return _texture;
}

void cr::draft_renderer::render()
{
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);
    glEnable(GL_DEPTH_TEST);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, _res_x, _res_y);
    glUseProgram(_program_handle);

    // Update uniforms
    _update_uniforms();

    for (const auto &mesh : _scene->get()->meshes())
    {
        glBindVertexArray(mesh.vao);
        glDrawArrays(GL_TRIANGLES, 0, mesh.indices);
    }
}

void cr::draft_renderer::_update_uniforms()
{
    const auto translation_location = glGetUniformLocation(_program_handle, "mvp");

    const auto projection =
      glm::perspective(
        glm::radians(_scene->get()->registry()->camera()->fov),
        static_cast<float>(_res_x) / _res_y,
        0.10f,
        1000.f
        );
    const auto view = glm::inverse(_scene->get()->registry()->camera()->mat4());

    // No model matrix *yet*
    const auto mvp = projection * view;

    glUniformMatrix4fv(translation_location, 1, GL_FALSE, glm::value_ptr(mvp));
}
