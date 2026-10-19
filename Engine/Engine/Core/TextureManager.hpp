#pragma once

#include <string>
#include <vector>

namespace rse {
class Texture;  // forward declaration for Texture

class TextureManager {
 public:
  static void Init();
  static void Add(Texture* texture);
  static Texture* Get(const std::string& name);
  static void Clean();

 private:
  TextureManager();
  static std::vector<Texture*> s_Textures;
};
}  // namespace rse