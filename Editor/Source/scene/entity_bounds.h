#pragma once

// ---------------------------------------------------------------------------
// EntityBounds -- how big an entity is on screen, in world space.
//
// Framing (the viewport's F), the selection outline and click-picking all need
// the same answer, and all three have to agree with what the renderer draws or
// they will not line up with the scene. The renderer builds
// T * Rx * Ry * Rz * S and applies it to the (+-size, 0) corners, so the shape
// is a rectangle that has been rotated and then scaled -- not an axis-aligned
// box, and not a square either once scale.x and scale.y differ. Two things are
// easy to get wrong and are stated here instead of at each call site: m_Size is
// a half extent (a sprite of size 40 is 80 world units across), and the scale
// is applied after the rotation.
//
// One caveat, recorded rather than hidden: the box only accounts for
// rotation.z. A sprite tilted about x or y is foreshortened by the projection
// and this rectangle will not match it. Nothing the editor can do today makes
// that tilt, and pretending otherwise would make F non-deterministic.
// ---------------------------------------------------------------------------

#include <array>
#include <core.h>
#include <optional>

#include <glm/glm.hpp>

#include <render/ortho_view.h>

struct EntityBounds {
  glm::vec2 center{0.0f, 0.0f};
  glm::vec2 half_extent{0.0f, 0.0f};  // already scaled
  float rotation_degrees{0.0f};       // transform.rotation.z
};

namespace editor_bounds {
// The entity's on-screen rectangle. Nothing when it has no sprite to
// measure, which is the same condition the renderer uses to skip it.
inline std::optional<EntityBounds> OfEntity(uint32_t entity) {
  // The selection is entt::null (EditorManager's INVALID_ENTITY_ID) when
  // nothing is selected, and entt would rather assert than answer for
  // that, so it is checked here rather than at every call site.
  if (entity == static_cast<uint32_t>(entt::null)) {
    return std::nullopt;
  }

  // A disabled entity is not drawn, so it has nothing to frame, outline or
  // pick. It is still selectable from the hierarchy.
  if (!rse::ECSManager::IsEntityEnabled(entity)) {
    return std::nullopt;
  }

  if (!rse::ECSManager::HasComponent<TransformComponent>(entity) ||
      !rse::ECSManager::HasComponent<SpriteComponent>(entity)) {
    return std::nullopt;
  }

  TransformComponent& transform = rse::ECSManager::GetComponent<TransformComponent>(entity);
  SpriteComponent& sprite = rse::ECSManager::GetComponent<SpriteComponent>(entity);

  if (sprite.m_sprite == nullptr) {
    return std::nullopt;
  }

  EntityBounds bounds;
  bounds.center = glm::vec2(transform.position.x, transform.position.y);
  bounds.rotation_degrees = transform.rotation.z;
  bounds.half_extent = sprite.m_sprite->GetSize() * glm::vec2(transform.scale.x, transform.scale.y);
  return bounds;
}

// The entity's world position. Everything selectable has one -- including an
// entity whose sprite has been removed, which is the case the outline draws
// a cross for, because there is no rectangle to draw instead.
inline std::optional<glm::vec2> PositionOf(uint32_t entity) {
  if (entity == static_cast<uint32_t>(entt::null)) {
    return std::nullopt;
  }

  if (!rse::ECSManager::IsEntityEnabled(entity)) {
    return std::nullopt;
  }

  if (!rse::ECSManager::HasComponent<TransformComponent>(entity)) {
    return std::nullopt;
  }

  TransformComponent& transform = rse::ECSManager::GetComponent<TransformComponent>(entity);
  return glm::vec2(transform.position.x, transform.position.y);
}

// The four corners of a bounds, in the order the outline draws them.
inline std::array<glm::vec2, 4> Corners(const EntityBounds& bounds) {
  return rse::OrientedBoxCorners(bounds.center, bounds.rotation_degrees, bounds.half_extent);
}

// The box around the rotated rectangle: what to frame, not what to draw.
inline rse::Bounds2D Aabb(const EntityBounds& bounds) { return rse::BoundsOfCorners(Corners(bounds)); }

// Everything in the scene that can be drawn, as one box. Nothing when the
// scene is empty -- which is different from a box at the origin, and is
// what keeps F from zooming off to nowhere.
inline std::optional<rse::Bounds2D> OfScene() {
  std::optional<rse::Bounds2D> total;

  rse::ECSManager::GetView<TransformComponent, SpriteComponent>().each(
      [&total](uint32_t entity, TransformComponent&, SpriteComponent&) {
        const std::optional<EntityBounds> bounds = OfEntity(entity);
        if (!bounds) {
          return;
        }

        const rse::Bounds2D boxed = Aabb(*bounds);
        total = total ? rse::UnionOf(*total, boxed) : boxed;
      });

  return total;
}
}  // namespace editor_bounds
