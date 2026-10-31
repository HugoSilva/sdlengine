#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include <physics/physics_material.h>

namespace rse {
// One loaded physics material: the asset's identity and its surface values.
struct PhysicsMaterialAsset {
  std::string id;
  std::string name;
  std::string path;
  PhysicsMaterial material;
};

// Owns the project's physics materials, one per asset. A rigid body stores the
// asset's id -- or a path when the file is not registered -- and PhysicsSystem
// resolves it here when the body is created.
//
// Init preloads the registry's physics materials so the first simulated frame
// does not read them during a step. GetOrLoad handles references that are not
// registered -- a path a scene still stores -- lazily. A missing or broken file
// still yields an asset with the default values, so a body is never left without
// a fixture.
class PhysicsMaterialManager {
 public:
  static void Init();

  // Takes ownership. Tests use this to add a material that was not loaded from
  // the registry.
  static void Add(std::unique_ptr<PhysicsMaterialAsset> material);

  // The material registered under an asset id or a name, or nullptr.
  static const PhysicsMaterialAsset* Get(const std::string& reference);

  // The material for `reference` -- an asset id, an asset name, or a path --
  // loading it the first time. A registered reference resolves through the
  // registry, so the material carries the asset id a scene saves. A missing or
  // broken file still yields an asset with the defaults.
  static const PhysicsMaterialAsset* GetOrLoad(const std::string& reference);

  // How many materials the manager owns. For tests and diagnostics.
  static std::size_t Count();

  // Destroys them all. Safe to call twice.
  static void Clean();

 private:
  PhysicsMaterialManager() {}
  static std::vector<std::unique_ptr<PhysicsMaterialAsset>> s_materials;
};
}  // namespace rse
