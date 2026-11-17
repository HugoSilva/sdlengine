#pragma once

#include <iostream>
#include <string>

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

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

  // The size the back buffer was last set to, in device pixels. BindDefaultFramebuffer
  // refreshes it from the window, so this is what the frame is actually drawn
  // into -- not what was requested when the window was created.
  glm::ivec2 GetDrawableSize() const { return m_drawable_size; }

 private:
  SDL_Event m_window_event;
  SDL_Window* m_window;
  SDL_GLContext m_context;

  std::string m_title;
  int m_width, m_height;
  bool m_vsync{true};
  glm::ivec2 m_drawable_size{0, 0};
};
}  // namespace rse