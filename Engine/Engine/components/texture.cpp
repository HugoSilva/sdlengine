#include "texture.h"

#ifdef EMSCRIPTEN
#include <SDL3/SDL_opengles2.h>
#else
#include <GL/glew.h>
#endif  // EMSCRIPTEN

#include <SDL3/SDL.h>

#include <render/font.h>
#include <utils/image.h>

namespace rse {
Texture::Texture(const std::string& name, const std::string& filename) : Texture("", name, filename) {}

Texture::Texture(const std::string& asset_id, const std::string& name, const std::string& filename)
    : m_asset_id(asset_id), m_name(name), m_filename(filename) {
  int mode = GL_RGB;
  // Image::loadImage hands us an owned surface; it must outlive the upload.
  SDL_Surface* surface = Image::LoadImage(m_filename.c_str());
  if (surface != nullptr) {
    Image::CheckImageMode(&mode, surface);
    m_width = surface->w;
    m_height = surface->h;
  }

  // Nothing to upload to without a current context: keep what was read (the
  // size, the name) and leave the id at zero, which is the same state a file
  // that is missing ends in. Headless loads -- the unit tests -- go through
  // this path instead of calling GL entry points with no context bound.
  if (SDL_GL_GetCurrentContext() == nullptr) {
    if (surface != nullptr) SDL_DestroySurface(surface);
    return;
  }

  unsigned int result;
  glGenTextures(1, &result);
  glBindTexture(GL_TEXTURE_2D, result);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, mode, m_width, m_height, 0, mode, GL_UNSIGNED_BYTE,
               surface != nullptr ? surface->pixels : nullptr);

  glBindTexture(GL_TEXTURE_2D, 0);

  if (surface != nullptr) SDL_DestroySurface(surface);

  m_id = result;
}

Texture::Texture(const std::string& name, Font* font, const std::string text, unsigned int color) : m_name(name) {
  SDL_Color text_color = {static_cast<unsigned char>(color), static_cast<unsigned char>(color >> 8),
                          static_cast<unsigned char>(color >> 16), static_cast<unsigned char>(color >> 24)};
  int mode = GL_RGB;
  SDL_Surface* surface = Image::LoadFont(font->GetFtFont(), text.c_str(), text_color);
  if (surface != nullptr) {
    Image::CheckImageMode(&mode, surface);
    m_width = surface->w;
    m_height = surface->h;
  }

  if (SDL_GL_GetCurrentContext() == nullptr) {
    if (surface != nullptr) SDL_DestroySurface(surface);
    return;
  }

  unsigned int result;
  glGenTextures(1, &result);
  glBindTexture(GL_TEXTURE_2D, result);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, mode, GL_UNSIGNED_BYTE,
               surface != nullptr ? surface->pixels : nullptr);

  glBindTexture(GL_TEXTURE_2D, 0);

  if (surface != nullptr) SDL_DestroySurface(surface);

  m_id = result;
}

Texture::~Texture() {
  // Zero means "never uploaded" -- there is no name to delete, and touching GL
  // at that point would be a call into a context that may not exist.
  if (m_id != 0) {
    glDeleteTextures(1, &m_id);
  }
}

void Texture::Bind() const { glBindTexture(GL_TEXTURE_2D, m_id); }

void Texture::Unbind() const { glBindTexture(GL_TEXTURE_2D, 0); }
}  // namespace rse
