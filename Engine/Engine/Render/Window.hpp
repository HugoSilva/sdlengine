#pragma once

#include <iostream>

#include <SDL3/SDL.h>

namespace rse {
class Window {
 public:
  Window(const char* title, int width, int height);
  ~Window();

  bool init();
  bool getRunning();
  SDL_Window* getWindow() { return m_Window; };
  SDL_GLContext getContext() { return m_Context; };

  // Makes the window's back buffer the render target again and resets the
  // GL viewport to the current pixel size (the window may have been
  // resized since the last frame).
  void bindDefaultFramebuffer();

 private:
  SDL_Event m_WindowEvent;
  SDL_Window* m_Window;
  SDL_GLContext m_Context;

  const char* m_Title;
  int m_Width, m_Height;
};
}  // namespace rse