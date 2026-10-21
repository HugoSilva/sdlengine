#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace rse {
class Texture;  // forward declaration for Texture

// Owns every texture in the project, one per asset. Components borrow them: a
// scene stores the asset's id (or, for a file with no registry entry, its name)
// and the manager hands out the one texture that reference resolved to, which
// is what makes twelve sprites sharing an image load it once instead of twelve
// times.
class TextureManager {
 public:
  static void Init();

  // Takes ownership. Callers construct a texture and hand it over; nothing
  // else may delete it afterwards.
  static void Add(std::unique_ptr<Texture> texture);

  // The texture registered under an asset id or a name, or nullptr.
  static Texture* Get(const std::string& reference);

  // The texture for `reference` -- an asset id, an asset name, or a bare path --
  // loading it the first time. A reference the asset registry knows resolves
  // through the registry, so the texture carries the asset id a scene saves.
  // Anything else is treated as a path, which is what keeps a scene written
  // before the registry loading. A reference is never null-resolved: a file that
  // cannot be read still yields a texture with id 0, which draws white, so a
  // scene referring to a missing image still opens.
  static Texture* GetOrLoad(const std::string& reference);

  // How many textures the manager owns. For tests and diagnostics.
  static std::size_t Count();

  // Destroys them all. Safe to call twice, and called before the registry goes
  // away so nothing is left pointing at a deleted texture.
  static void Clean();

 private:
  TextureManager() {}
  static std::vector<std::unique_ptr<Texture>> s_textures;
};
}  // namespace rse
