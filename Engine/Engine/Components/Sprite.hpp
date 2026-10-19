#pragma once

#include <cereal/cereal.hpp>
#include <cereal/types/polymorphic.hpp>
#include <glm/glm.hpp>

#include "../Render/Renderable2D.hpp"
#include "../Render/Renderer2D.hpp"

namespace rse {
class Sprite : public rse::Renderable2D {
 public:
  Sprite();
  Sprite(glm::vec3 position, glm::vec2 size, unsigned int color);
  Sprite(glm::vec3 position, glm::vec2 size, Texture* texture);

  void Submit(rse::Renderer2D* renderer) {};
  glm::vec3& position;

  template <typename Archive>
  void serialize(Archive& archive) {
    archive(cereal::base_class<rse::Renderable2D>(this), cereal::make_nvp("Position", position));
  }
};
}  // namespace rse

CEREAL_REGISTER_TYPE(rse::Sprite);

CEREAL_REGISTER_POLYMORPHIC_RELATION(rse::Renderable2D, rse::Sprite)