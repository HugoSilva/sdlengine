#include "physics_material_manager.h"

#include <core.h>
#include <filesystem>
#include <utility>

#include <core/asset_registry.h>
#include <core/physics_material_file.h>
#include <utils/logger.h>

namespace rse {
namespace {
std::string FileStem(const std::filesystem::path& path) {
  const std::string stem = path.stem().string();
  return stem.empty() ? path.filename().string() : stem;
}
}  // namespace

std::vector<std::unique_ptr<PhysicsMaterialAsset>> PhysicsMaterialManager::s_materials;

void PhysicsMaterialManager::Init() {
  if (!s_materials.empty()) {
    return;
  }

  for (const AssetRegistry::Asset& asset : AssetRegistry::All()) {
    if (asset.type != "physics_material" || asset.path.empty()) {
      continue;
    }

    GetOrLoad(asset.id);
  }
}

void PhysicsMaterialManager::Add(std::unique_ptr<PhysicsMaterialAsset> material) {
  if (material != nullptr) {
    s_materials.push_back(std::move(material));
  }
}

const PhysicsMaterialAsset* PhysicsMaterialManager::Get(const std::string& reference) {
  if (reference.empty()) {
    return nullptr;
  }

  // The common case is an id or a display name, which is a direct compare with
  // no registry work. A scene can also store a registry name or a path, so only
  // resolve those when the direct pass misses.
  for (const std::unique_ptr<PhysicsMaterialAsset>& material : s_materials) {
    if (material->id == reference || material->name == reference) {
      return material.get();
    }
  }

  std::string asset_id;

  if (const AssetRegistry::Asset* asset = AssetRegistry::FindReference(reference)) {
    if (asset->type == "physics_material") {
      asset_id = asset->id;
    }
  }

  const std::string project_path = Core::ResolveProjectPath(reference);
  const std::string resource_path = Core::ResolveResource(reference);

  for (const std::unique_ptr<PhysicsMaterialAsset>& material : s_materials) {
    if ((!asset_id.empty() && material->id == asset_id) ||
        (!material->path.empty() && (material->path == project_path || material->path == resource_path))) {
      return material.get();
    }
  }

  return nullptr;
}

const PhysicsMaterialAsset* PhysicsMaterialManager::GetOrLoad(const std::string& reference) {
  // An empty reference is "no material", not a material called nothing.
  if (reference.empty()) {
    return nullptr;
  }

  if (const PhysicsMaterialAsset* existing = Get(reference)) {
    return existing;
  }

  std::string asset_id;
  std::string name = reference;
  std::string path;

  if (const AssetRegistry::Asset* asset = AssetRegistry::FindReference(reference)) {
    // A registered asset: the id is what the scene saves and the registry's path
    // is where the file is.
    if (asset->type != "physics_material") {
      Logger::Warning("asset '" + reference + "' is a " + asset->type + ", not a physics material");
      return nullptr;
    }

    asset_id = asset->id;
    name = asset->name;
    path = Core::ResolveProjectPath(asset->path);
  } else {
    // Not registered: the reference is a path. Try it relative to the project
    // first, then relative to the asset root.
    path = Core::ResolveProjectPath(reference);

    if (!std::filesystem::exists(path)) {
      path = Core::ResolveResource(reference);
    }
  }

  PhysicsMaterialFile file;
  std::string error;

  if (!PhysicsMaterialFile::Load(path, &file, &error)) {
    // Cache the failure too: a body created with a missing material must not
    // retry the disk every time it is rebuilt. The defaults keep the body
    // simulatable.
    Logger::Warning("physics material '" + name + "' could not be loaded: " + error);
    auto material = std::make_unique<PhysicsMaterialAsset>();
    material->id = asset_id;
    material->name = name;
    material->path = path;
    PhysicsMaterialAsset* created = material.get();
    s_materials.push_back(std::move(material));
    return created;
  }

  if (!file.name.empty()) {
    name = file.name;
  } else if (name.empty()) {
    name = FileStem(path);
  }

  auto material = std::make_unique<PhysicsMaterialAsset>();
  material->id = asset_id;
  material->name = name;
  material->path = path;
  material->material = file.material;
  PhysicsMaterialAsset* created = material.get();
  s_materials.push_back(std::move(material));
  return created;
}

std::size_t PhysicsMaterialManager::Count() { return s_materials.size(); }

void PhysicsMaterialManager::Clean() { s_materials.clear(); }
}  // namespace rse
