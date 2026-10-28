#include "material_shader.h"

#include <utility>

#include <components/shader.h>

namespace rse {
MaterialShader::MaterialShader(std::string asset_id, std::string name, std::string asset_path,
                               std::unique_ptr<Shader> shader)
    : m_asset_id(std::move(asset_id)),
      m_name(std::move(name)),
      m_asset_path(std::move(asset_path)),
      m_shader(std::move(shader)) {}

MaterialShader::~MaterialShader() = default;
}  // namespace rse
