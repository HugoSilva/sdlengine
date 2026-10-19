#include "FontManager.hpp"

#include "../Utils/Logger.hpp"
#include "Font.hpp"

namespace rse {
std::vector<Font*> FontManager::m_fonts;

void FontManager::Init() {
  // SDL3_ttf returns bool now (true == success).
  if (!TTF_Init()) {
    Logger::Error("SDL_ttf could not initialize, " + std::string(SDL_GetError()));
  }
}

void FontManager::Add(Font* font) { m_fonts.push_back(font); }

Font* FontManager::Get() { return m_fonts[0]; }

Font* FontManager::Get(const std::string& name) {
  for (Font* font : m_fonts) {
    if (font->GetName() == name) return font;
  }
  // TODO: Maybe return a default font instead?
  return nullptr;
}

Font* FontManager::Get(const std::string& name, unsigned int size) {
  for (Font* font : m_fonts) {
    if (font->GetSize() == size && font->GetName() == name) return font;
  }
  // TODO: Maybe return a default font instead?
  return nullptr;
}

void FontManager::Clean() {
  for (unsigned int i = 0; i < m_fonts.size(); i++) delete m_fonts[i];

  TTF_Quit();
}
}  // namespace rse