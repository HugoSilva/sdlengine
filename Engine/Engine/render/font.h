#pragma once

#include <string>

#include <SDL3_ttf/SDL_ttf.h>
#include <glm/glm.hpp>

namespace rse {
class Font {
 public:
  Font(std::string name, std::string filename, int size);
  ~Font();

  inline TTF_Font* GetFtFont() const { return m_font; }
  inline const std::string& GetName() const { return m_name; }
  inline const std::string& GetFileName() const { return m_filename; }
  inline const int GetSize() const { return m_size; }

 private:
  TTF_Font* m_font;
  unsigned int m_size;
  std::string m_name;
  std::string m_filename;
};
}  // namespace rse