#pragma once

#include <memory>
#include <vector>

#include <cereal/cereal.hpp>
#include <glm/glm.hpp>

#include "../Components/Texture.hpp"
#include "Renderer2D.hpp"

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

class Renderable2D {
 public:
  Renderable2D(glm::vec3 position, glm::vec2 size, unsigned int color)
      : m_Position(position), m_Size(size), m_Color(color) {
    SetUvDefaults();
  }

  virtual ~Renderable2D() {}

  virtual void Submit(Renderer2D* renderer) const {
    // renderer->submit(this);
  }

  inline const glm::vec3& GetPosition() const { return m_Position; }
  inline const glm::vec2& GetSize() const { return m_Size; }
  inline void SetColor(unsigned int color) { m_Color = color; }
  inline const unsigned int& GetColor() const { return m_Color; }
  inline const std::vector<glm::vec2>& GetUv() const { return m_UV; }
  inline const unsigned int GetTId() const { return m_Texture.get() == nullptr ? 0 : m_Texture.get()->GetId(); }
  inline const std::string GetTextureName() const {
    return m_Texture.get() == nullptr ? "" : m_Texture.get()->GetName();
  }

  template <typename Archive>
  void serialize(Archive& archive) {
    archive(cereal::make_nvp("Size", m_Size), cereal::make_nvp("Color", m_Color),
            cereal::make_nvp("Texture", m_Texture));
  }

 private:
  void SetUvDefaults() {
    m_UV.push_back(glm::vec2(0, 1));
    m_UV.push_back(glm::vec2(0, 0));
    m_UV.push_back(glm::vec2(1, 0));
    m_UV.push_back(glm::vec2(1, 1));
  }

 protected:
  Renderable2D() { SetUvDefaults(); }

  glm::vec3 m_Position;
  glm::vec2 m_Size;
  unsigned int m_Color{0x00};
  std::vector<glm::vec2> m_UV;
  std::shared_ptr<Texture> m_Texture{nullptr};
};
}  // namespace rse