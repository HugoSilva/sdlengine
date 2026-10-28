#include "material_manager.h"

#include <core.h>
#include <filesystem>
#include <utility>

#include <SDL3/SDL.h>

#include <components/material_shader.h>
#include <components/shader.h>
#include <core/asset_registry.h>
#include <core/material_file.h>
#include <utils/logger.h>

namespace rse {
namespace {
std::string FileStem(const std::filesystem::path& path) {
  const std::string stem = path.stem().string();
  return stem.empty() ? path.filename().string() : stem;
}

std::string ResolveShaderPath(const std::string& declared, const char* fallback) {
  if (!declared.empty()) {
    const std::string project_path = Core::ResolveProjectPath(declared);

    if (std::filesystem::exists(project_path)) {
      return project_path;
    }

    const std::string resource_path = Core::ResolveResource(declared);

    if (std::filesystem::exists(resource_path)) {
      return resource_path;
    }
  }

  return Core::ResolveResource(fallback);
}
}  // namespace

std::vector<std::unique_ptr<MaterialShader>> MaterialManager::s_materials;

void MaterialManager::Init() {
  if (!s_materials.empty()) return;

  for (const AssetRegistry::Asset& asset : AssetRegistry::All()) {
    if (asset.type != "material" || asset.path.empty()) continue;
    GetOrLoad(asset.id);
  }
}

void MaterialManager::Add(std::unique_ptr<MaterialShader> material) {
  if (material != nullptr) {
    s_materials.push_back(std::move(material));
  }
}

MaterialShader* MaterialManager::Get(const std::string& reference) {
  if (reference.empty()) {
    return nullptr;
  }

  // The common case is an id or a display name, which is a direct compare with
  // no registry work. A scene can also store a registry name or a legacy path,
  // so only resolve those when the direct pass misses.
  for (const std::unique_ptr<MaterialShader>& material : s_materials) {
    if (material->GetAssetId() == reference || material->GetName() == reference) {
      return material.get();
    }
  }

  std::string asset_id;

  if (const AssetRegistry::Asset* asset = AssetRegistry::FindReference(reference)) {
    if (asset->type == "material") {
      asset_id = asset->id;
    }
  }

  const std::string project_path = Core::ResolveProjectPath(reference);
  const std::string resource_path = Core::ResolveResource(reference);

  for (const std::unique_ptr<MaterialShader>& material : s_materials) {
    if ((!asset_id.empty() && material->GetAssetId() == asset_id) ||
        (!material->GetAssetPath().empty() &&
         (material->GetAssetPath() == project_path || material->GetAssetPath() == resource_path))) {
      return material.get();
    }
  }
  return nullptr;
}

MaterialShader* MaterialManager::GetOrLoad(const std::string& reference) {
  // An empty reference is "no material", not a material called nothing.
  if (reference.empty()) {
    return nullptr;
  }

  if (MaterialShader* existing = Get(reference)) {
    return existing;
  }

  std::string asset_id;
  std::string name = reference;
  std::string path;

  if (const AssetRegistry::Asset* asset = AssetRegistry::FindReference(reference)) {
    // A registered asset: the id is what the scene saves and the registry's path
    // is where the file is.
    if (asset->type != "material") {
      Logger::Warning("asset '" + reference + "' is a " + asset->type + ", not a material");
      return nullptr;
    }

    asset_id = asset->id;
    name = asset->name;
    path = Core::ResolveProjectPath(asset->path);
  } else {
    // Not registered: the reference is a path. Try it relative to the project
    // first, then relative to the asset root, which is what older files store.
    path = Core::ResolveProjectPath(reference);

    if (!std::filesystem::exists(path)) {
      path = Core::ResolveResource(reference);
    }
  }

  MaterialFile file;
  std::string error;

  if (!MaterialFile::Load(path, &file, &error)) {
    // Cache the failure too: a frame that draws a sprite with a missing
    // material must not retry the disk every frame.
    Logger::Warning("material '" + name + "' could not be loaded: " + error);
    auto material = std::make_unique<MaterialShader>(asset_id, name, path, nullptr);
    MaterialShader* created = material.get();
    s_materials.push_back(std::move(material));
    return created;
  }

  if (!file.name.empty()) {
    name = file.name;
  } else if (name.empty()) {
    name = FileStem(path);
  }

  std::unique_ptr<Shader> shader;

  // Compiling needs a current context. Without one (a headless test, a tool
  // inspecting a project), the material still parses and still answers with its
  // name and path; it just has no program to draw with.
  if (SDL_GL_GetCurrentContext() != nullptr) {
    try {
      const std::string vertex_path = ResolveShaderPath(file.vertex_shader, "Default.vert");
      const std::string fragment_path = ResolveShaderPath(file.fragment_shader, "Default.frag");
      auto compiled = std::make_unique<Shader>(vertex_path.c_str(), fragment_path.c_str());

      if (compiled->GetShaderId() != 0) {
        shader = std::move(compiled);
      } else {
        Logger::Warning("material '" + name + "' did not produce a shader program");
      }
    } catch (const std::exception& exception) {
      Logger::Warning("material '" + name + "' could not be compiled: " + exception.what());
    } catch (...) {
      Logger::Warning("material '" + name + "' could not be compiled");
    }
  }

  auto material = std::make_unique<MaterialShader>(asset_id, name, path, std::move(shader));
  MaterialShader* created = material.get();
  s_materials.push_back(std::move(material));
  return created;
}

std::size_t MaterialManager::Count() { return s_materials.size(); }

void MaterialManager::Clean() { s_materials.clear(); }
}  // namespace rse
