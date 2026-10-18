#pragma once

// ---------------------------------------------------------------------------
// OrthoView -- the pure 2D view maths shared by the engine camera and the
// editor camera.
//
// No GL and no editor concepts: free functions over plain values, which is what
// lets it be unit-tested (UnitTests links Engine, so pure maths has to live in
// an engine header to be reachable). It exists because two places now need to
// answer the same questions -- "what world rectangle does this target show?",
// "where does this world point land in the panel?", "what is a nice grid step
// at this zoom?" -- and they have to agree, or the grid, the outline and the
// picking will not line up with the scene underneath them.
//
// The one rule that matters: the view rectangle is derived from the *shape of
// the target*. A fixed 1280x720 rectangle stretched onto a differently shaped
// target is exactly what made the editor viewport distort the scene.
// ---------------------------------------------------------------------------

#include <algorithm>
#include <array>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace rse
{
	struct OrthoView
	{
		glm::vec2 center{ 0.0f, 0.0f };
		glm::vec2 halfExtent{ 1.0f, 1.0f };

		glm::vec2 min() const { return center - halfExtent; }
		glm::vec2 max() const { return center + halfExtent; }
		glm::vec2 size() const { return halfExtent * 2.0f; }
		float aspect() const { return halfExtent.y > 0.0f ? halfExtent.x / halfExtent.y : 1.0f; }
	};

	struct Bounds2D
	{
		glm::vec2 min{ 0.0f, 0.0f };
		glm::vec2 max{ 0.0f, 0.0f };

		glm::vec2 center() const { return (min + max) * 0.5f; }
		glm::vec2 size() const { return max - min; }
	};

	// The zoom range the viewport has always used.
	inline constexpr float kOrthoMinZoom = 0.05f;
	inline constexpr float kOrthoMaxZoom = 20.0f;

	// The view rectangle for a given zoom, shaped like targetSize. referenceSize
	// is what a zoom of 1 shows vertically -- it is what the renderer has always
	// framed, so at a 16:9 target this is bit for bit the old behaviour.
	inline OrthoView makeOrthoView(glm::vec2 center, float zoom, glm::vec2 targetSize, glm::vec2 referenceSize)
	{
		const float safeZoom = std::clamp(zoom, kOrthoMinZoom, kOrthoMaxZoom);
		const float height = referenceSize.y / safeZoom;
		const float targetAspect = targetSize.y > 0.0f ? targetSize.x / targetSize.y : 1.0f;
		const float width = height * targetAspect;

		OrthoView view;
		view.center = center;
		view.halfExtent = glm::vec2(width * 0.5f, height * 0.5f);
		return view;
	}

	inline glm::mat4 makeOrthoProjection(const OrthoView& view)
	{
		const glm::vec2 min = view.min();
		const glm::vec2 max = view.max();

		return glm::ortho(min.x, max.x, min.y, max.y, -1.0f, 1.0f);
	}

	// Device pixels, y down from the top-left of the target: the convention both
	// ImGui draw lists and mouse input use.
	inline glm::vec2 worldToViewport(const OrthoView& view, glm::vec2 world, glm::vec2 deviceSize)
	{
		const glm::vec2 size = view.size();
		const glm::vec2 min = view.min();

		const float u = size.x > 0.0f ? (world.x - min.x) / size.x : 0.5f;
		const float v = size.y > 0.0f ? (world.y - min.y) / size.y : 0.5f;

		return glm::vec2(u * deviceSize.x, (1.0f - v) * deviceSize.y);
	}

	inline glm::vec2 viewportToWorld(const OrthoView& view, glm::vec2 pixel, glm::vec2 deviceSize)
	{
		const glm::vec2 size = view.size();
		const glm::vec2 min = view.min();

		const float u = deviceSize.x > 0.0f ? pixel.x / deviceSize.x : 0.5f;
		const float v = deviceSize.y > 0.0f ? 1.0f - (pixel.y / deviceSize.y) : 0.5f;

		return glm::vec2(min.x + u * size.x, min.y + v * size.y);
	}

	// Centres the view on the bounds and grows it until they fit, keeping the
	// target's aspect ratio. padding is a fraction of the fitted half extent.
	inline OrthoView fitToBounds(const OrthoView& view, const Bounds2D& worldBounds, float padding)
	{
		// A point (or an empty selection) would otherwise give a zero-sized view
		// and an infinite zoom. The caller clamps the zoom; this only has to
		// avoid the division.
		glm::vec2 half = glm::max(worldBounds.size() * 0.5f, glm::vec2(1e-4f));

		const float aspect = view.aspect();
		if (half.x > half.y * aspect)
		{
			half.y = half.x / aspect;
		}
		else
		{
			half.x = half.y * aspect;
		}

		OrthoView fitted = view;
		fitted.center = worldBounds.center();
		fitted.halfExtent = half * (1.0f + std::max(padding, 0.0f));
		return fitted;
	}

	// The smallest "nice" step -- 1, 2, 5, 10, 20, 50, ... -- that still leaves
	// at least minPixels between grid lines. Monotonic in worldPerPixel, so
	// zooming out never makes the grid finer.
	inline float niceStep(float worldPerPixel, float minPixels)
	{
		const float minWorld = worldPerPixel * minPixels;
		if (!(minWorld > 0.0f))
		{
			return 1.0f;
		}

		const float exponent = std::floor(std::log10(minWorld));
		const float base = std::pow(10.0f, exponent);
		const float mantissa = minWorld / base;

		float nice = 10.0f;
		if (mantissa <= 1.0f) { nice = 1.0f; }
		else if (mantissa <= 2.0f) { nice = 2.0f; }
		else if (mantissa <= 5.0f) { nice = 5.0f; }

		return nice * base;
	}

	// The renderer builds every sprite as translate * rotate(x,y,z) * scale
	// applied to the (+-halfExtent, 0) corners, so a box that matches what is on
	// screen is a rotated, non-uniformly scaled rectangle -- never an
	// axis-aligned one.
	inline std::array<glm::vec2, 4> orientedBoxCorners(glm::vec2 center, float rotationDegrees, glm::vec2 halfExtent)
	{
		const float radians = glm::radians(rotationDegrees);
		const float c = std::cos(radians);
		const float s = std::sin(radians);

		const glm::vec2 alongX{ c * halfExtent.x, s * halfExtent.x };
		const glm::vec2 alongY{ -s * halfExtent.y, c * halfExtent.y };

		return {
			center - alongX - alongY,
			center + alongX - alongY,
			center + alongX + alongY,
			center - alongX + alongY
		};
	}

	inline bool pointInOrientedBox(glm::vec2 point, glm::vec2 center, float rotationDegrees, glm::vec2 halfExtent)
	{
		const glm::vec2 delta = point - center;

		// Rotate the point into the box's own frame rather than rotating the
		// box: same test, four fewer sin/cos.
		const float radians = glm::radians(-rotationDegrees);
		const float c = std::cos(radians);
		const float s = std::sin(radians);
		const glm::vec2 local{ delta.x * c - delta.y * s, delta.x * s + delta.y * c };

		return std::abs(local.x) <= halfExtent.x && std::abs(local.y) <= halfExtent.y;
	}
}
