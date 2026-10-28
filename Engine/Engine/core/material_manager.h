#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace rse {
class MaterialShader;

// Owns the project's material shaders, one per asset. A sprite stores the
// material's asset id -- or a path when the file is not registered -- and the
// manager resolves it to the one compiled program every sprite using that
// material shares.
//
// Init preloads the registry's materials so the first frame does not compile
// them during a draw. GetOrLoad handles references that are not registered --
// a path an older scene still stores -- lazily. A headless tool or a unit test
// gets a parsed MaterialShader with no program instead, so material files stay
// inspectable without a window.
class MaterialManager {
 public:
  static void Init();

  // Takes ownership. Tests and the sample project use this to add a material
  // that was not loaded from the registry.
  static void Add(std::unique_ptr<MaterialShader> material);

  // The material registered under an asset id or a name, or nullptr.
  static MaterialShader* Get(const std::string& reference);

  // The material for `reference` -- an asset id, an asset name, or a path --
  // loading it the first time. A registered reference resolves through the
  // registry, so the material carries the asset id a scene saves. A missing or
  // broken file still yields a MaterialShader object with no compiled program;
  // renderers fall back to the scene shader rather than failing the frame.
  static MaterialShader* GetOrLoad(const std::string& reference);

  // How many materials the manager owns. For tests and diagnostics.
  static std::size_t Count();

  // Destroys them all. Safe to call twice, and called before the GL context
  // goes away so the programs are deleted while there is still a context.
  static void Clean();

 private:
  MaterialManager() {}
  static std::vector<std::unique_ptr<MaterialShader>> s_materials;
};
}  // namespace rse
