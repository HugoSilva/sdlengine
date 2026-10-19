#include "sprite.h"

namespace rse {
Sprite::Sprite() : Renderable2D(glm::vec3(0.0f), glm::vec2(1.0f), 0), position(m_position) {}

Sprite::Sprite(glm::vec3 position, glm::vec2 size, unsigned int color)
    : Renderable2D(position, size, color), position(m_position) {}

Sprite::Sprite(glm::vec3 position, glm::vec2 size, Texture* texture)
    : Renderable2D(position, size, 0xffffffff), position(m_position) {
  m_texture.reset(texture);
}
}  // namespace rse