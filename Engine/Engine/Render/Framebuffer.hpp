#pragma once

#ifdef EMSCRIPTEN
#define GL_GLEXT_PROTOTYPES
#include <SDL3/SDL_opengles2.h>
#else
#include <GL/glew.h>
#endif  // EMSCRIPTEN

namespace rse {
// -----------------------------------------------------------------------
// Offscreen colour + depth render target.
//
// Used by the editor viewport: the scene is drawn into this instead of the
// window's back buffer, and the colour attachment is then displayed with
// ImGui::Image. That indirection is what makes the scene a panel like any
// other -- resizable, dockable, and eventually one of several viewports.
//
// Note the texture is bottom-up like every GL texture, so an ImGui::Image
// of it has to flip its V coordinates.
// -----------------------------------------------------------------------
class Framebuffer {
 public:
  Framebuffer() = default;
  ~Framebuffer();

  Framebuffer(const Framebuffer&) = delete;
  Framebuffer& operator=(const Framebuffer&) = delete;

  // (Re)allocates the attachments when the size changes and does nothing
  // otherwise, so it is safe to call every frame.
  void resize(int width, int height);

  // Binds the target and sets the GL viewport to its size.
  void bind() const;

  // Back to the window's back buffer.
  static void unbind();

  unsigned int getColorTexture() const { return m_ColorTexture; }
  int getWidth() const { return m_Width; }
  int getHeight() const { return m_Height; }
  bool isValid() const { return m_Framebuffer != 0; }

 private:
  void destroy();

  unsigned int m_Framebuffer{0};
  unsigned int m_ColorTexture{0};
  unsigned int m_DepthBuffer{0};
  int m_Width{0};
  int m_Height{0};
};
}  // namespace rse
