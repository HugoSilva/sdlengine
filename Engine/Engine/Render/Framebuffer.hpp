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
  void Resize(int width, int height);

  // Binds the target and sets the GL viewport to its size.
  void Bind() const;

  // Back to the window's back buffer.
  static void Unbind();

  unsigned int GetColorTexture() const { return m_ColorTexture; }
  int GetWidth() const { return m_Width; }
  int GetHeight() const { return m_Height; }
  bool IsValid() const { return m_Framebuffer != 0; }

 private:
  void Destroy();

  unsigned int m_Framebuffer{0};
  unsigned int m_ColorTexture{0};
  unsigned int m_DepthBuffer{0};
  int m_Width{0};
  int m_Height{0};
};
}  // namespace rse
