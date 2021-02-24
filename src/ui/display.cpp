#include "display.h"

Display::Display()
{
}

void Display::start()
{
}

void Display::stop()
{
}

void Display::register_prerender_callback(
  const std::function<PreRenderResponse(const Display &)> &callback)
{
    _prerender_callback = callback;
}

void Display::register_postrender_callback(
  const std::function<PostRenderResult(const Display &)> &callback)
{
    _postrender_callback = callback
}
