#pragma once

#include <iostream>
#include <string>

#include <SDL3/SDL.h>

namespace rse {
class Window {
 public:
  Window(const char* title, int width, int height);
  ~Window();

  bool Init();
  bool GetRunning();
  SDL_Window* GetWindow() { return m_window; };
  SDL_GLContext GetContext() { return m_context; };

  // Changes the title after the window exists. The editor uses it to name the
  // open project and to mark unsaved work.
  void SetTitle(const std::string& title);

  // Makes the window's back buffer the render target again and resets the
  // GL viewport to the current pixel size (the window may have been
  // resized since the last frame).
  void BindDefaultFramebuffer();

 private:
  SDL_Event m_window_event;
  SDL_Window* m_window;
  SDL_GLContext m_context;

  std::string m_title;
  int m_width, m_height;
};
}  // namespace rse