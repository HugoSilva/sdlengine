#include "texture_manager.h"

#include <components/texture.h>
#include <utils/logger.h>

namespace rse {
std::vector<Texture*> TextureManager::s_textures;

void TextureManager::Init() {
  // SDL3_image no longer needs IMG_Init()/IMG_Quit(); formats boot
  // themselves on first use, so there is nothing to set up here.
}

void TextureManager::Add(Texture* texture) { s_textures.push_back(texture); }

Texture* TextureManager::Get(const std::string& name) {
  for (Texture* texture : s_textures) {
    if (texture->GetName() == name) return texture;
  }
  return nullptr;
}

void TextureManager::Clean() {
  for (unsigned int i = 0; i < s_textures.size(); i++) delete s_textures[i];
}
}  // namespace rse
