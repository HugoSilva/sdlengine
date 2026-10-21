#pragma once

#include <memory>
#include <vector>

#include <cereal/cereal.hpp>
#include <glm/glm.hpp>

#include "renderer_2d.h"
#include <components/texture.h>
#include <core/texture_manager.h>

namespace rse {
struct VertexData {
  glm::vec3 vertex;
  glm::vec2 uv;
  float tid;
  unsigned int color;
};

// Sprites carry their tint as an unsigned int with red in the low byte.
// InspectorPanel unpacks it for the colour picker; this is the matching
// packer, so the layout is stated once instead of re-derived at every call
// site that has an r/g/b/a in hand.
inline constexpr unsigned int MakeColor(unsigned int r, unsigned int g, unsigned int b, unsigned int a = 255) {
  return (r & 0xFF) | ((g & 0xFF) << 8) | ((b & 0xFF) << 16) | ((a & 0xFF) << 24);
}

// Textures are owned by TextureManager; a component only borrows one. The
// shared_ptr stays because of the existing null checks and the member type,
// but its deleter does nothing -- deleting a texture the manager owns would be
// a double free the moment two sprites share it.
inline std::shared_ptr<Texture> BorrowTexture(Texture* texture) {
  return std::shared_ptr<Texture>(texture, [](Texture*) {});
}

class Renderable2D {
 public:
  Renderable2D(glm::vec3 position, glm::vec2 size, unsigned int color)
      : m_position(position), m_size(size), m_color(color) {
    SetUvDefaults();
  }

  virtual ~Renderable2D() {}

  virtual void Submit(Renderer2D* renderer) const {
    // renderer->submit(this);
  }

  inline const glm::vec3& GetPosition() const { return m_position; }
  inline const glm::vec2& GetSize() const { return m_size; }
  inline void SetColor(unsigned int color) { m_color = color; }
  inline const unsigned int& GetColor() const { return m_color; }
  inline const std::vector<glm::vec2>& GetUv() const { return m_uv; }
  inline const unsigned int GetTId() const { return m_texture.get() == nullptr ? 0 : m_texture.get()->GetId(); }
  inline const std::string GetTextureName() const {
    return m_texture.get() == nullptr ? "" : m_texture.get()->GetName();
  }
  inline const std::string GetTextureAssetId() const {
    return m_texture.get() == nullptr ? "" : m_texture.get()->GetAssetId();
  }

  template <typename Archive>
  void serialize(Archive& archive) {
    archive(cereal::make_nvp("Size", m_size), cereal::make_nvp("Color", m_color));

    // A scene stores a *reference* to the texture, never a copy of it: the
    // asset registry's id when the texture has one, or the name when it came
    // from a bare path (a file with no registry entry, or a test). The manager
    // owns one texture per reference, so twelve sprites that use one asset
    // share one upload. Loading asks the manager for the reference, which
    // resolves an id, a name or a path and is the only place a texture is
    // created.
    if constexpr (Archive::is_saving::value) {
      const std::string reference =
          m_texture == nullptr ? ""
                               : (m_texture->GetAssetId().empty() ? m_texture->GetName() : m_texture->GetAssetId());
      archive(cereal::make_nvp("Texture", reference));
    } else {
      std::string reference;
      archive(cereal::make_nvp("Texture", reference));
      m_texture = reference.empty() ? nullptr : BorrowTexture(TextureManager::GetOrLoad(reference));
    }
  }

 private:
  void SetUvDefaults() {
    m_uv.push_back(glm::vec2(0, 1));
    m_uv.push_back(glm::vec2(0, 0));
    m_uv.push_back(glm::vec2(1, 0));
    m_uv.push_back(glm::vec2(1, 1));
  }

 protected:
  Renderable2D() { SetUvDefaults(); }

  glm::vec3 m_position;
  glm::vec2 m_size;
  unsigned int m_color{0x00};
  std::vector<glm::vec2> m_uv;
  std::shared_ptr<Texture> m_texture{nullptr};
};
}  // namespace rse