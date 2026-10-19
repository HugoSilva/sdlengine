#pragma once

// ---------------------------------------------------------------------------
// Draws the editor's world-space grid into the scene pass, underneath the
// sprites.
//
// It is real geometry rather than an ImGui overlay: the grid lives in world
// space so it pans and zooms with the scene for free, and it is submitted in the
// pre-scene phase so the sprites land on top of it.
//
// Each line is an untextured quad one *device* pixel thick, snapped to the pixel
// grid, so it rasterises as a crisp single pixel -- the viewport framebuffer has
// no MSAA to fall back on. The batch is bounded by the minimum line spacing
// (12px), so even a 1920px wide framebuffer is a couple of hundred quads.
// ---------------------------------------------------------------------------

#include <Core.h>
#include <cmath>

#include <glm/glm.hpp>

#include "../Camera/EditorCamera.hpp"
#include "../EditorSettings.hpp"
#include <Render/OrthoView.hpp>

class GridRenderer {
 public:
  // Alpha matters: the sprite shader blends, so these composite over the
  // renderer's clear colour instead of hiding it.
  struct Colors {
    unsigned int minor{rse::MakeColor(150, 150, 150, 40)};
    unsigned int major{rse::MakeColor(160, 160, 160, 90)};
    unsigned int axis_x{rse::MakeColor(200, 60, 60, 190)};  // the line y = 0
    unsigned int axis_y{rse::MakeColor(80, 200, 80, 190)};  // the line x = 0
  };

  const Colors& GetColors() const { return m_colors; }
  Colors& GetColors() { return m_colors; }

  // The minimum spacing the *drawn* step is allowed to fall to, in device
  // pixels. Below this the grid reads as a solid sheet.
  static constexpr float kMinLineSpacing = 12.0f;

  // Safety net only: the 12px minimum already bounds the count at
  // deviceWidth/12, but a degenerate view must not be able to spin the loop.
  static constexpr int kMaxLines = 4096;

  void Submit(rse::Renderer2D* renderer, const EditorCamera& camera, const EditorSettings::Grid& settings) const {
    if (renderer == nullptr || !settings.visible) {
      return;
    }

    const rse::OrthoView& view = camera.GetView();
    const float world_per_pixel = camera.GetWorldPerDevicePixel();

    if (!(world_per_pixel > 0.0f)) {
      return;
    }

    const float step = rse::GridStep(settings.base, world_per_pixel, kMinLineSpacing);
    const float half = world_per_pixel * 0.5f;

    // Every column before every row: the two sets overlap at the
    // intersections and blending the same alpha twice there is visible, so it
    // is better that it happens uniformly.
    const int first_x = static_cast<int>(std::floor(view.Min().x / step));
    const int last_x = static_cast<int>(std::ceil(view.Max().x / step));
    if (last_x - first_x <= kMaxLines) {
      for (int i = first_x; i <= last_x; ++i) {
        if (i == 0) {
          continue;  // the axis goes on top, below
        }
        SubmitVertical(renderer, view, static_cast<float>(i) * step, half, world_per_pixel,
                       (i % kMajorEvery == 0) ? m_colors.major : m_colors.minor);
      }
    }

    const int first_y = static_cast<int>(std::floor(view.Min().y / step));
    const int last_y = static_cast<int>(std::ceil(view.Max().y / step));
    if (last_y - first_y <= kMaxLines) {
      for (int i = first_y; i <= last_y; ++i) {
        if (i == 0) {
          continue;
        }
        SubmitHorizontal(renderer, view, static_cast<float>(i) * step, half, world_per_pixel,
                         (i % kMajorEvery == 0) ? m_colors.major : m_colors.minor);
      }
    }

    // The world axes, last so they sit on top of the rest of the grid.
    SubmitVertical(renderer, view, 0.0f, half, world_per_pixel, m_colors.axis_y);
    SubmitHorizontal(renderer, view, 0.0f, half, world_per_pixel, m_colors.axis_x);
  }

 private:
  // A major line every 5 minor ones. Any multiple of the step is a multiple of
  // grid.base, so major lines stay on the snapping lattice too.
  static constexpr int kMajorEvery = 5;

  // Snaps a world coordinate to the nearest whole device pixel, so a one pixel
  // wide quad covers exactly one pixel instead of straddling two.
  static float AlignToPixel(float world, float view_min, float world_per_pixel) {
    return view_min + std::round((world - view_min) / world_per_pixel) * world_per_pixel;
  }

  static void SubmitVertical(rse::Renderer2D* renderer, const rse::OrthoView& view, float world_x, float half,
                             float world_per_pixel, unsigned int color) {
    TransformComponent transform;
    transform.position = glm::vec3(AlignToPixel(world_x, view.Min().x, world_per_pixel), view.center.y, 0.0f);

    renderer->SubmitQuad(transform, glm::vec2(half, view.half_extent.y), color);
  }

  static void SubmitHorizontal(rse::Renderer2D* renderer, const rse::OrthoView& view, float world_y, float half,
                               float world_per_pixel, unsigned int color) {
    TransformComponent transform;
    transform.position = glm::vec3(view.center.x, AlignToPixel(world_y, view.Min().y, world_per_pixel), 0.0f);

    renderer->SubmitQuad(transform, glm::vec2(view.half_extent.x, half), color);
  }

  Colors m_colors;
};
