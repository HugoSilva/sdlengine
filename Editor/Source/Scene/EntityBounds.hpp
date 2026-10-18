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
#include <optional>

#include <glm/glm.hpp>

#include <Core.h>
#include <Render/OrthoView.hpp>

struct EntityBounds
{
	glm::vec2 center{ 0.0f, 0.0f };
	glm::vec2 halfExtent{ 0.0f, 0.0f };   // already scaled
	float rotationDegrees{ 0.0f };        // transform.rotation.z
};

namespace EditorBounds
{
	// The entity's on-screen rectangle. Nothing when it has no sprite to
	// measure, which is the same condition the renderer uses to skip it.
	inline std::optional<EntityBounds> ofEntity(uint32_t entity)
	{
		// The selection is entt::null (EditorManager's INVALID_ENTITY_ID) when
		// nothing is selected, and entt would rather assert than answer for
		// that, so it is checked here rather than at every call site.
		if (entity == static_cast<uint32_t>(entt::null))
		{
			return std::nullopt;
		}

		if (!rse::ECSManager::hasComponent<TransformComponent>(entity) ||
			!rse::ECSManager::hasComponent<SpriteComponent>(entity))
		{
			return std::nullopt;
		}

		TransformComponent& transform = rse::ECSManager::getComponent<TransformComponent>(entity);
		SpriteComponent& sprite = rse::ECSManager::getComponent<SpriteComponent>(entity);

		if (sprite.m_Sprite == nullptr)
		{
			return std::nullopt;
		}

		EntityBounds bounds;
		bounds.center = glm::vec2(transform.position.x, transform.position.y);
		bounds.rotationDegrees = transform.rotation.z;
		bounds.halfExtent = sprite.m_Sprite->getSize() * glm::vec2(transform.scale.x, transform.scale.y);
		return bounds;
	}

	// The four corners of a bounds, in the order the outline draws them.
	inline std::array<glm::vec2, 4> corners(const EntityBounds& bounds)
	{
		return rse::orientedBoxCorners(bounds.center, bounds.rotationDegrees, bounds.halfExtent);
	}

	// The box around the rotated rectangle: what to frame, not what to draw.
	inline rse::Bounds2D aabb(const EntityBounds& bounds)
	{
		return rse::boundsOfCorners(corners(bounds));
	}

	// Everything in the scene that can be drawn, as one box. Nothing when the
	// scene is empty -- which is different from a box at the origin, and is
	// what keeps F from zooming off to nowhere.
	inline std::optional<rse::Bounds2D> ofScene()
	{
		std::optional<rse::Bounds2D> total;

		rse::ECSManager::getView<TransformComponent, SpriteComponent>().each(
			[&total](uint32_t entity, TransformComponent&, SpriteComponent&)
			{
				const std::optional<EntityBounds> bounds = ofEntity(entity);
				if (!bounds)
				{
					return;
				}

				const rse::Bounds2D boxed = aabb(*bounds);
				total = total ? rse::unionOf(*total, boxed) : boxed;
			});

		return total;
	}
}
