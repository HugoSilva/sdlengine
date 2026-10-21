#include "texture_manager.h"

#include <core.h>
#include <utility>

#include <components/texture.h>
#include <core/asset_registry.h>
#include <utils/logger.h>

namespace rse {
std::vector<std::unique_ptr<Texture>> TextureManager::s_textures;

void TextureManager::Init() {
  // SDL3_image no longer needs IMG_Init()/IMG_Quit(); formats boot
  // themselves on first use, so there is nothing to set up here.
}

void TextureManager::Add(std::unique_ptr<Texture> texture) { s_textures.push_back(std::move(texture)); }

Texture* TextureManager::Get(const std::string& reference) {
  for (const std::unique_ptr<Texture>& texture : s_textures) {
    // An id and a name are both references to the same texture: a scene that
    // stores the id and an inspector that shows the name have to land on it.
    if (texture->GetAssetId() == reference || texture->GetName() == reference) return texture.get();
  }
  return nullptr;
}

Texture* TextureManager::GetOrLoad(const std::string& reference) {
  // An empty reference is "no texture", not a texture called nothing.
  if (reference.empty()) {
    return nullptr;
  }

  if (Texture* existing = Get(reference)) {
    return existing;
  }

  std::string asset_id;
  std::string name = reference;
  std::string path;

  if (const AssetRegistry::Asset* asset = AssetRegistry::FindReference(reference)) {
    // A registered asset: the id is what the scene will save, the name is what
    // the inspector shows, and the registry's path is where the file is.
    asset_id = asset->id;
    name = asset->name;
    path = Core::ResolveProjectPath(asset->path);
  } else {
    // Not registered: the reference is a path, relative to the asset root,
    // exactly as it was before the registry existed. This is the fallback that
    // keeps an older scene loading.
    path = Core::ResolveResource(reference);
  }

  auto texture = std::make_unique<Texture>(asset_id, name, path);

  // One warning per reference, because this only runs the first time one is
  // asked for. The texture stays in the manager at id 0, so the sprites that
  // refer to it draw white instead of failing the load.
  if (texture->GetId() == 0) {
    Logger::Warning("texture '" + name + "' could not be loaded; sprites using it draw white");
  }

  Texture* created = texture.get();
  s_textures.push_back(std::move(texture));
  return created;
}

std::size_t TextureManager::Count() { return s_textures.size(); }

void TextureManager::Clean() { s_textures.clear(); }
}  // namespace rse
