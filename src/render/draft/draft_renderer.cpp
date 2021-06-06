#include "draft_renderer.h"

cr::draft_renderer::draft_renderer(
  uint64_t res_x,
  uint64_t res_y,
  std::unique_ptr<cr::scene> *scene)
:
 _res_x(res_x), _res_y(res_y), _scene(scene)
{
    glGenFramebuffers(1, &_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, _framebuffer);

    glGenTextures(1, &_texture);

    glBindTexture(GL_TEXTURE_2D, _texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _res_x, _res_y, 0, GL_RGBA8, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glGenRenderbuffers(1, &_depth_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _depth_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _res_x, _res_y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depth_buffer);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _texture, 0);

    GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, draw_buffers);

}

GLuint cr::draft_renderer::rendered_texture() const
{
    return _framebuffer;
}

void cr::draft_renderer::render(uint64_t res_x, uint64_t res_y)
{
}

void cr::draft_renderer::update_keyboard_inputs(std::array<char, 300> keys)
{
}
