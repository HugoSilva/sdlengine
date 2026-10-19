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
    unsigned int axisX{rse::MakeColor(200, 60, 60, 190)};  // the line y = 0
    unsigned int axisY{rse::MakeColor(80, 200, 80, 190)};  // the line x = 0
  };

  const Colors& GetColors() const { return m_Colors; }
  Colors& GetColors() { return m_Colors; }

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
    const float worldPerPixel = camera.GetWorldPerDevicePixel();

    if (!(worldPerPixel > 0.0f)) {
      return;
    }

    const float step = rse::GridStep(settings.base, worldPerPixel, kMinLineSpacing);
    const float half = worldPerPixel * 0.5f;

    // Every column before every row: the two sets overlap at the
    // intersections and blending the same alpha twice there is visible, so it
    // is better that it happens uniformly.
    const int firstX = static_cast<int>(std::floor(view.Min().x / step));
    const int lastX = static_cast<int>(std::ceil(view.Max().x / step));
    if (lastX - firstX <= kMaxLines) {
      for (int i = firstX; i <= lastX; ++i) {
        if (i == 0) {
          continue;  // the axis goes on top, below
        }
        SubmitVertical(renderer, view, static_cast<float>(i) * step, half, worldPerPixel,
                       (i % kMajorEvery == 0) ? m_Colors.major : m_Colors.minor);
      }
    }

    const int firstY = static_cast<int>(std::floor(view.Min().y / step));
    const int lastY = static_cast<int>(std::ceil(view.Max().y / step));
    if (lastY - firstY <= kMaxLines) {
      for (int i = firstY; i <= lastY; ++i) {
        if (i == 0) {
          continue;
        }
        SubmitHorizontal(renderer, view, static_cast<float>(i) * step, half, worldPerPixel,
                         (i % kMajorEvery == 0) ? m_Colors.major : m_Colors.minor);
      }
    }

    // The world axes, last so they sit on top of the rest of the grid.
    SubmitVertical(renderer, view, 0.0f, half, worldPerPixel, m_Colors.axisY);
    SubmitHorizontal(renderer, view, 0.0f, half, worldPerPixel, m_Colors.axisX);
  }

 private:
  // A major line every 5 minor ones. Any multiple of the step is a multiple of
  // grid.base, so major lines stay on the snapping lattice too.
  static constexpr int kMajorEvery = 5;

  // Snaps a world coordinate to the nearest whole device pixel, so a one pixel
  // wide quad covers exactly one pixel instead of straddling two.
  static float AlignToPixel(float world, float viewMin, float worldPerPixel) {
    return viewMin + std::round((world - viewMin) / worldPerPixel) * worldPerPixel;
  }

  static void SubmitVertical(rse::Renderer2D* renderer, const rse::OrthoView& view, float worldX, float half,
                             float worldPerPixel, unsigned int color) {
    TransformComponent transform;
    transform.position = glm::vec3(AlignToPixel(worldX, view.Min().x, worldPerPixel), view.center.y, 0.0f);

    renderer->SubmitQuad(transform, glm::vec2(half, view.halfExtent.y), color);
  }

  static void SubmitHorizontal(rse::Renderer2D* renderer, const rse::OrthoView& view, float worldY, float half,
                               float worldPerPixel, unsigned int color) {
    TransformComponent transform;
    transform.position = glm::vec3(view.center.x, AlignToPixel(worldY, view.Min().y, worldPerPixel), 0.0f);

    renderer->SubmitQuad(transform, glm::vec2(view.halfExtent.x, half), color);
  }

  Colors m_Colors;
};
