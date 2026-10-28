#pragma once

#include <memory>
#include <string>

namespace rse {
class Shader;

// One material asset, resolved through MaterialManager. The manager owns the
// object and the compiled program; a sprite stores only the asset reference,
// so twenty sprites using one material share one program exactly like they
// share one texture.
class MaterialShader {
 public:
  MaterialShader(std::string asset_id, std::string name, std::string asset_path, std::unique_ptr<Shader> shader);
  ~MaterialShader();

  MaterialShader(const MaterialShader&) = delete;
  MaterialShader& operator=(const MaterialShader&) = delete;

  const std::string& GetAssetId() const { return m_asset_id; }
  const std::string& GetName() const { return m_name; }
  const std::string& GetAssetPath() const { return m_asset_path; }

  // Null when the file could not be read or compiled, or when the manager was
  // asked for a material without a GL context. Renderers fall back to their
  // scene shader in that case, so a broken material does not hide the scene.
  Shader* GetShader() const { return m_shader.get(); }
  bool IsLoaded() const { return m_shader != nullptr; }

 private:
  std::string m_asset_id;
  std::string m_name;
  std::string m_asset_path;
  std::unique_ptr<Shader> m_shader;
};
}  // namespace rse
