#pragma once

#include <string>

#include <SDL3_ttf/SDL_ttf.h>
#include <glm/glm.hpp>

namespace rse {
class Font {
 public:
  Font(std::string name, std::string filename, int size);
  ~Font();

  inline TTF_Font* GetFtFont() const { return m_Font; }
  inline const std::string& GetName() const { return m_Name; }
  inline const std::string& GetFileName() const { return m_Filename; }
  inline const int GetSize() const { return m_Size; }

 private:
  TTF_Font* m_Font;
  unsigned int m_Size;
  std::string m_Name;
  std::string m_Filename;
};
}  // namespace rse