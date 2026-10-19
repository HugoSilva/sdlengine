#include "FontManager.hpp"

#include "../Utils/Logger.hpp"
#include "Font.hpp"

namespace rse {
std::vector<Font*> FontManager::m_Fonts;

void FontManager::init() {
  // SDL3_ttf returns bool now (true == success).
  if (!TTF_Init()) {
    Logger::error("SDL_ttf could not initialize, " + std::string(SDL_GetError()));
  }
}

void FontManager::add(Font* font) { m_Fonts.push_back(font); }

Font* FontManager::get() { return m_Fonts[0]; }

Font* FontManager::get(const std::string& name) {
  for (Font* font : m_Fonts) {
    if (font->getName() == name) return font;
  }
  // TODO: Maybe return a default font instead?
  return nullptr;
}

Font* FontManager::get(const std::string& name, unsigned int size) {
  for (Font* font : m_Fonts) {
    if (font->getSize() == size && font->getName() == name) return font;
  }
  // TODO: Maybe return a default font instead?
  return nullptr;
}

void FontManager::clean() {
  for (unsigned int i = 0; i < m_Fonts.size(); i++) delete m_Fonts[i];

  TTF_Quit();
}
}  // namespace rse