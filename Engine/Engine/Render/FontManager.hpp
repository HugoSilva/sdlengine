#pragma once

#include <string>
#include <vector>

namespace rse {
class Font;  // forward declaration for Font

class FontManager {
 public:
  static void Init();
  static void Add(Font* font);
  static Font* Get();
  static Font* Get(const std::string& name);
  static Font* Get(const std::string& name, unsigned int size);
  static void Clean();

 private:
  FontManager() {}
  static std::vector<Font*> m_fonts;
};
}  // namespace rse