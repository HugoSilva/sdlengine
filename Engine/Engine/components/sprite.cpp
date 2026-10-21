#include "sprite.h"

namespace rse {
Sprite::Sprite() : Renderable2D(glm::vec3(0.0f), glm::vec2(1.0f), 0) {}

Sprite::Sprite(glm::vec3 position, glm::vec2 size, unsigned int color) : Renderable2D(position, size, color) {}

Sprite::Sprite(glm::vec3 position, glm::vec2 size, Texture* texture) : Renderable2D(position, size, 0xffffffff) {
  // Borrowed, not owned: the texture belongs to TextureManager and outlives
  // this sprite (see BorrowTexture).
  m_texture = BorrowTexture(texture);
}
}  // namespace rse