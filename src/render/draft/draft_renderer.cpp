#include "draft_renderer.h"

cr::draft_renderer::draft_renderer(
  uint64_t                    res_x,
  uint64_t                    res_y,
  std::unique_ptr<cr::scene> *scene)
    : _res_x(res_x), _res_y(res_y), _scene(scene)
{
    glGenFramebuffers(1, &_framebuffer);

    glGenTextures(1, &_texture);
    glBindTexture(GL_TEXTURE_2D, _texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenRenderbuffers(1, &_rbo);
    _setup_required();

    _vertex_handle =
      cr::opengl::create_shader("./assets/app/shaders/draft_mode.vert", GL_VERTEX_SHADER);

    _fragment_handle =
      cr::opengl::create_shader("./assets/app/shaders/draft_mode.frag", GL_FRAGMENT_SHADER);

    _program_handle = cr::opengl::create_program(_vertex_handle, _fragment_handle);

    _background_shader_handle = cr::opengl::create_shader(
      "./assets/app/shaders/draft_mode_background.comp",
      GL_COMPUTE_SHADER);

    _background_program_handle = cr::opengl::create_program(_background_shader_handle);
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

    if (_scene->get()->skybox_handle().has_value())
    {
        glUseProgram(_background_program_handle);
        // Run the compute background program to setup the background for the image
        glUniformMatrix4fv(
          glGetUniformLocation(_background_program_handle, "camera"),
          1,
          GL_FALSE,
          glm::value_ptr(_scene->get()->registry()->camera()->mat4()));

        glUniform2i(glGetUniformLocation(_background_program_handle, "scene_size"), _res_x, _res_y);

        glUniform1f(
          glGetUniformLocation(_background_program_handle, "aspect_correction"),
          static_cast<float>(_res_x) / _res_y);

        glUniform1f(
          glGetUniformLocation(_background_program_handle, "fov"),
          _scene->get()->registry()->camera()->fov);

        glBindTexture(GL_TEXTURE_2D, _texture);
        glClearTexImage(_texture, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glBindImageTexture(0, _texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _scene->get()->skybox_handle().value());

        glDispatchCompute(
          static_cast<int>(glm::ceil(_res_x / 8)),
          static_cast<int>(glm::ceil(_res_y / 8)),
          1);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

    glViewport(0, 0, _res_x, _res_y);

    glUseProgram(_program_handle);

    for (const auto entity : _scene->get()
                               ->registry()
                               ->entities.view<cr::entity::model_gpu_data, cr::entity::instances>())
    {
        const auto meshes =
          _scene->get()->registry()->entities.get<cr::entity::model_gpu_data>(entity).meshes;
        const auto instances =
          _scene->get()->registry()->entities.get<cr::entity::instances>(entity);

        for (const auto &mesh : meshes)
        {
            if (mesh.material.info.tex.has_value())
            {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, mesh.texture);
            }

            glBindVertexArray(mesh.vao);

            for (const auto &transform : instances.transforms)
            {
                _update_uniforms(transform);
                glDrawArrays(GL_TRIANGLES, 0, mesh.indices);
            }
        }
    }
}

void cr::draft_renderer::_update_uniforms(const glm::mat4 &model)
{
    const auto mvp_location = glGetUniformLocation(_program_handle, "mvp");

    const auto camera = _scene->get()->registry()->camera();

    auto projection = glm::mat4();
    switch (camera->current_mode)
    {
    case camera::mode::perspective:
        projection = glm::perspectiveLH(
          glm::radians(_scene->get()->registry()->camera()->fov),
          static_cast<float>(_res_x) / _res_y,
          0.10f,
          10000.f);
        break;

    case camera::mode::orthographic:
    {
        const auto scale = camera->scale;

        projection = glm::orthoLH(-scale, scale, -scale, scale, 0.01f, 10000.f);
    }
    break;
    }
    const auto view = glm::inverse(_scene->get()->registry()->camera()->mat4());

    // No model matrix *yet*
    // Model matrix!
    const auto mvp = projection * view * model;

    glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp));
}

void cr::draft_renderer::set_resolution(uint64_t res_x, uint64_t res_y)
{
    _res_x = res_x;
    _res_y = res_y;

    _setup_required();
}

void cr::draft_renderer::_setup_required()
{
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);

    glBindTexture(GL_TEXTURE_2D, _texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _res_x, _res_y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _texture, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, _rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, _res_x, _res_y);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cr::logger::error("Framebuffer is not complete");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_FRAMEBUFFER, 0);
}
