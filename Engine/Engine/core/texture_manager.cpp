#include "texture_manager.h"

#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include <utility>

#include <SDL3/SDL.h>

#ifdef EMSCRIPTEN
#include <SDL3/SDL_opengles2.h>
#else
#include <GL/glew.h>
#endif  // EMSCRIPTEN

#include <core.h>

#include <components/texture.h>
#include <core/asset_registry.h>
#include <render/texture_atlas.h>
#include <utils/image.h>
#include <utils/logger.h>

namespace rse {
namespace {
constexpr int kAtlasMaxSize = 2048;
constexpr int kAtlasPadding = 1;

void DestroySurfaces(std::vector<SDL_Surface*>* surfaces) {
  for (SDL_Surface* surface : *surfaces) {
    if (surface != nullptr) SDL_DestroySurface(surface);
  }
  surfaces->clear();
}

// Loads each unique texture path once, converts it to the one format the atlas
// understands, and leaves it owned by `surfaces`. A missing or unreadable file
// is represented by a null entry so the caller can fall back to an individual
// texture with the same "draws white" behaviour as before.
std::vector<SDL_Surface*> LoadAtlasSurfaces(const std::vector<std::string>& paths) {
  std::vector<SDL_Surface*> surfaces;
  surfaces.reserve(paths.size());
  for (const std::string& path : paths) {
    SDL_Surface* loaded = Image::LoadSurface(path.c_str());
    if (loaded == nullptr) {
      surfaces.push_back(nullptr);
      continue;
    }

    SDL_Surface* converted = SDL_ConvertSurface(loaded, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(loaded);
    surfaces.push_back(converted);
  }
  return surfaces;
}

void CopySurfaceIntoAtlas(SDL_Surface* surface, const AtlasRegion& region, std::vector<std::uint8_t>* pixels,
                          int page_width) {
  if (surface == nullptr || !SDL_LockSurface(surface)) return;

  const auto* source = static_cast<const std::uint8_t*>(surface->pixels);
  const int source_pitch = surface->pitch;
  const int copy_width = std::min(surface->w, region.width);
  const int copy_height = std::min(surface->h, region.height);

  for (int row = 0; row < copy_height; ++row) {
    const std::uint8_t* src = source + static_cast<std::size_t>(row) * source_pitch;
    std::uint8_t* dst = pixels->data() + (static_cast<std::size_t>(region.y + row) * page_width + region.x) * 4;
    std::copy(src, src + static_cast<std::size_t>(copy_width) * 4, dst);
  }

  SDL_UnlockSurface(surface);
}
}  // namespace

std::vector<std::unique_ptr<Texture>> TextureManager::s_textures;

void TextureManager::Init() {
  if (!s_textures.empty()) return;

  // There is nothing to upload to without a current context. This is the same
  // path a headless tool takes; tests construct textures directly.
  if (SDL_GL_GetCurrentContext() == nullptr) return;

  std::vector<const AssetRegistry::Asset*> texture_assets;
  std::vector<std::string> paths;
  std::unordered_map<std::string, std::size_t> path_index;

  for (const AssetRegistry::Asset& asset : AssetRegistry::All()) {
    if (asset.type != "texture" || asset.path.empty()) continue;

    const std::string path = Core::ResolveProjectPath(asset.path);
    if (path_index.find(path) == path_index.end()) {
      path_index.emplace(path, paths.size());
      paths.push_back(path);
    }
    texture_assets.push_back(&asset);
  }

  if (texture_assets.empty()) return;

  std::vector<SDL_Surface*> surfaces = LoadAtlasSurfaces(paths);
  bool can_pack = true;
  for (SDL_Surface* surface : surfaces) {
    if (surface == nullptr) {
      can_pack = false;
      break;
    }
  }

  GLint max_texture_size = 0;
  glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);
  const int max_size = std::min(kAtlasMaxSize, static_cast<int>(max_texture_size));
  AtlasPacker packer(max_size, kAtlasPadding);
  std::vector<AtlasRegion> regions(surfaces.size());

  if (can_pack) {
    for (std::size_t i = 0; i < surfaces.size(); ++i) {
      if (!packer.Add(surfaces[i]->w, surfaces[i]->h, &regions[i])) {
        can_pack = false;
        break;
      }
    }
  }

  // A project with one unreadable image or one image too large for the page
  // falls back to independent textures. The scene still opens; it just does not
  // get the batching win.
  if (!can_pack) {
    DestroySurfaces(&surfaces);
    for (const AssetRegistry::Asset* asset : texture_assets) {
      const std::string path = Core::ResolveProjectPath(asset->path);
      s_textures.push_back(std::make_unique<Texture>(asset->id, asset->name, path));
    }
    return;
  }

  const int page_width = packer.GetPageWidth();
  const int page_height = packer.GetPageHeight();
  std::vector<std::uint8_t> pixels(static_cast<std::size_t>(page_width) * page_height * 4, 0);
  for (std::size_t i = 0; i < surfaces.size(); ++i) {
    CopySurfaceIntoAtlas(surfaces[i], regions[i], &pixels, page_width);
  }
  DestroySurfaces(&surfaces);

  auto page = std::make_shared<TexturePage>();
  glGenTextures(1, &page->id);
  glBindTexture(GL_TEXTURE_2D, page->id);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, page_width, page_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
  glBindTexture(GL_TEXTURE_2D, 0);

  for (const AssetRegistry::Asset* asset : texture_assets) {
    const std::string path = Core::ResolveProjectPath(asset->path);
    const std::size_t index = path_index[path];
    const AtlasRegion& region = regions[index];
    const glm::vec4 uv(static_cast<float>(region.x) / page_width, static_cast<float>(region.y) / page_height,
                       static_cast<float>(region.x + region.width) / page_width,
                       static_cast<float>(region.y + region.height) / page_height);
    s_textures.push_back(std::make_unique<Texture>(asset->id, asset->name, page, uv, region.width, region.height));
  }

  Logger::Log("texture atlas: packed " + std::to_string(texture_assets.size()) + " textures into " +
              std::to_string(page_width) + "x" + std::to_string(page_height));
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
