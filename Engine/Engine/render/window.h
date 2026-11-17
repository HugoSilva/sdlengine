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

  // Hands presentation to the driver (on) or lets the loop present as fast as
  // it can (off). The project decides; the window starts in step with the
  // display, so nothing tears while a project is still being read.
  void SetVSync(bool enabled);
  bool GetVSync() const { return m_vsync; }

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
  bool m_vsync{true};
};
}  // namespace rse