#include "TextureManager.hpp"

#include "../Components/Texture.hpp"
#include "../Utils/Logger.hpp"

namespace rse {
std::vector<Texture*> TextureManager::s_Textures;

void TextureManager::Init() {
  // SDL3_image no longer needs IMG_Init()/IMG_Quit(); formats boot
  // themselves on first use, so there is nothing to set up here.
}

void TextureManager::Add(Texture* texture) { s_Textures.push_back(texture); }

Texture* TextureManager::Get(const std::string& name) {
  for (Texture* texture : s_Textures) {
    if (texture->GetName() == name) return texture;
  }
  return nullptr;
}

void TextureManager::Clean() {
  for (unsigned int i = 0; i < s_Textures.size(); i++) delete s_Textures[i];
}
}  // namespace rse
