#pragma once

#include <cereal/cereal.hpp>
#include <glm/glm.hpp>

#include <components/sprite.h>

struct SpriteComponent {
  SpriteComponent() {};

  SpriteComponent(rse::Sprite* sprite) : m_sprite(sprite) {};

  std::shared_ptr<rse::Sprite> m_sprite;

  template <typename Archive>
  void serialize(Archive& archive) {
    archive(cereal::make_nvp("Sprite", m_sprite));
  }
};
