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

namespace rse {
struct OrthoView {
  glm::vec2 center{0.0f, 0.0f};
  glm::vec2 half_extent{1.0f, 1.0f};

  glm::vec2 Min() const { return center - half_extent; }
  glm::vec2 Max() const { return center + half_extent; }
  glm::vec2 Size() const { return half_extent * 2.0f; }
  float Aspect() const { return half_extent.y > 0.0f ? half_extent.x / half_extent.y : 1.0f; }
};

struct Bounds2D {
  glm::vec2 min{0.0f, 0.0f};
  glm::vec2 max{0.0f, 0.0f};

  glm::vec2 Center() const { return (min + max) * 0.5f; }
  glm::vec2 Size() const { return max - min; }
};

// The zoom range the viewport has always used.
inline constexpr float kOrthoMinZoom = 0.05f;
inline constexpr float kOrthoMaxZoom = 20.0f;

// The view rectangle for a given zoom, shaped like targetSize. referenceSize
// is what a zoom of 1 shows vertically -- it is what the renderer has always
// framed, so at a 16:9 target this is bit for bit the old behaviour.
inline OrthoView MakeOrthoView(glm::vec2 center, float zoom, glm::vec2 target_size, glm::vec2 reference_size) {
  const float safe_zoom = std::clamp(zoom, kOrthoMinZoom, kOrthoMaxZoom);
  const float height = reference_size.y / safe_zoom;
  const float target_aspect = target_size.y > 0.0f ? target_size.x / target_size.y : 1.0f;
  const float width = height * target_aspect;

  OrthoView view;
  view.center = center;
  view.half_extent = glm::vec2(width * 0.5f, height * 0.5f);
  return view;
}

inline glm::mat4 MakeOrthoProjection(const OrthoView& view) {
  const glm::vec2 min = view.Min();
  const glm::vec2 max = view.Max();

  return glm::ortho(min.x, max.x, min.y, max.y, -1.0f, 1.0f);
}

// Device pixels, y down from the top-left of the target: the convention both
// ImGui draw lists and mouse input use.
inline glm::vec2 WorldToViewport(const OrthoView& view, glm::vec2 world, glm::vec2 device_size) {
  const glm::vec2 size = view.Size();
  const glm::vec2 min = view.Min();

  const float u = size.x > 0.0f ? (world.x - min.x) / size.x : 0.5f;
  const float v = size.y > 0.0f ? (world.y - min.y) / size.y : 0.5f;

  return glm::vec2(u * device_size.x, (1.0f - v) * device_size.y);
}

inline glm::vec2 ViewportToWorld(const OrthoView& view, glm::vec2 pixel, glm::vec2 device_size) {
  const glm::vec2 size = view.Size();
  const glm::vec2 min = view.Min();

  const float u = device_size.x > 0.0f ? pixel.x / device_size.x : 0.5f;
  const float v = device_size.y > 0.0f ? 1.0f - (pixel.y / device_size.y) : 0.5f;

  return glm::vec2(min.x + u * size.x, min.y + v * size.y);
}

// Centres the view on the bounds and grows it until they fit, keeping the
// target's aspect ratio. padding is a fraction of the fitted half extent.
inline OrthoView FitToBounds(const OrthoView& view, const Bounds2D& world_bounds, float padding) {
  // A point (or an empty selection) would otherwise give a zero-sized view
  // and an infinite zoom. The caller clamps the zoom; this only has to
  // avoid the division.
  glm::vec2 half = glm::max(world_bounds.Size() * 0.5f, glm::vec2(1e-4f));

  const float aspect = view.Aspect();
  if (half.x > half.y * aspect) {
    half.y = half.x / aspect;
  } else {
    half.x = half.y * aspect;
  }

  OrthoView fitted = view;
  fitted.center = world_bounds.Center();
  fitted.half_extent = half * (1.0f + std::max(padding, 0.0f));
  return fitted;
}

// The smallest "nice" step -- 1, 2, 5, 10, 20, 50, ... -- that still leaves
// at least minPixels between grid lines. Monotonic in worldPerPixel, so
// zooming out never makes the grid finer.
inline float NiceStep(float world_per_pixel, float min_pixels) {
  const float min_world = world_per_pixel * min_pixels;
  if (!(min_world > 0.0f)) {
    return 1.0f;
  }

  const float exponent = std::floor(std::log10(min_world));
  const float base = std::pow(10.0f, exponent);
  const float mantissa = min_world / base;

  float nice = 10.0f;
  if (mantissa <= 1.0f) {
    nice = 1.0f;
  } else if (mantissa <= 2.0f) {
    nice = 2.0f;
  } else if (mantissa <= 5.0f) {
    nice = 5.0f;
  }

  return nice * base;
}

// An upper bound for the doubling loop below: past this the grid is a
// meaningless number of world units across, and the loop must not be able to
// run away on a degenerate worldPerPixel.
inline constexpr float kMaxGridStep = 1.0e7f;

// The drawn grid step: the smallest power-of-two multiple of base that still
// leaves at least minPixels between lines. Doubling rather than jumping to a
// nice number keeps every line on a multiple of base -- the lattice snapping
// uses. A grid you can see and a grid you snap to have to be the same grid.
inline float GridStep(float base, float world_per_pixel, float min_pixels) {
  float step = base > 0.0f ? base : 1.0f;
  const float minimum = NiceStep(world_per_pixel, min_pixels);

  while (step < minimum && step < kMaxGridStep) {
    step *= 2.0f;
  }

  return step;
}

// The renderer builds every sprite as translate * rotate(x,y,z) * scale
// applied to the (+-halfExtent, 0) corners, so a box that matches what is on
// screen is a rotated, non-uniformly scaled rectangle -- never an
// axis-aligned one.
inline std::array<glm::vec2, 4> OrientedBoxCorners(glm::vec2 center, float rotation_degrees, glm::vec2 half_extent) {
  const float radians = glm::radians(rotation_degrees);
  const float c = std::cos(radians);
  const float s = std::sin(radians);

  const glm::vec2 along_x{c * half_extent.x, s * half_extent.x};
  const glm::vec2 along_y{-s * half_extent.y, c * half_extent.y};

  return {center - along_x - along_y, center + along_x - along_y, center + along_x + along_y,
          center - along_x + along_y};
}

inline bool PointInOrientedBox(glm::vec2 point, glm::vec2 center, float rotation_degrees, glm::vec2 half_extent) {
  const glm::vec2 delta = point - center;

  // Rotate the point into the box's own frame rather than rotating the
  // box: same test, four fewer sin/cos.
  const float radians = glm::radians(-rotation_degrees);
  const float c = std::cos(radians);
  const float s = std::sin(radians);
  const glm::vec2 local{delta.x * c - delta.y * s, delta.x * s + delta.y * c};

  return std::abs(local.x) <= half_extent.x && std::abs(local.y) <= half_extent.y;
}

// The axis-aligned box around four corners. A rotated entity draws as a
// rotated rectangle, so the box that frames it has to come from its corners
// rather than from its half extent -- otherwise rotating something would
// make F frame less than the thing it is framing.
inline Bounds2D BoundsOfCorners(const std::array<glm::vec2, 4>& corners) {
  Bounds2D bounds{corners[0], corners[0]};

  for (const glm::vec2& corner : corners) {
    bounds.min = glm::min(bounds.min, corner);
    bounds.max = glm::max(bounds.max, corner);
  }

  return bounds;
}

// The smallest box containing both. Folding a scene (or a selection) into
// one rectangle to frame is this, repeated.
inline Bounds2D UnionOf(const Bounds2D& a, const Bounds2D& b) {
  return Bounds2D{glm::min(a.min, b.min), glm::max(a.max, b.max)};
}

// A box test for the gizmo handles, which are axis-aligned rectangles in
// screen space: two comparisons per axis, in either direction.
inline bool PointInBox(glm::vec2 point, glm::vec2 min, glm::vec2 max) {
  return point.x >= min.x && point.x <= max.x && point.y >= min.y && point.y <= max.y;
}

// Round to the nearest multiple of step, which is what Shift does to a
// translate (the grid step), a rotate (15 degrees) and a scale (0.1). A
// non-positive step means "do not snap", so a caller can hand a setting
// through without testing it first.
inline float SnapToStep(float value, float step) {
  if (step <= 0.0f) {
    return value;
  }

  return std::round(value / step) * step;
}
}  // namespace rse
