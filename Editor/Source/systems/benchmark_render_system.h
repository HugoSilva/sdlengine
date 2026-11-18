#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

#include <components/sprite.h>
#include <ecs/components/transform.h>
#include <ecs/systems/base_system.h>
#include <render/renderable_2d.h>
#include <render/renderer_2d.h>

// ---------------------------------------------------------------------------
// Submits a synthetic grid of sprites straight to the renderer.
//
// It exists to give the performance work a repeatable load. The cost it
// measures is the batcher's -- submit, sort, cull, vertex build, upload -- not
// the ECS iteration's, so it deliberately bypasses the registry: it never
// touches the open scene, so it cannot dirty, save or undo anything, and a
// benchmark run leaves the project exactly as it was.
//
// The grid is built for the view the engine starts with, 1280x720 world units
// centred on (640, 360), so every quad is on screen and nothing is culled. The
// count changes the cell size, not the framed area: raising it packs more, not
// bigger, sprites into the same rectangle.
// ---------------------------------------------------------------------------
class BenchmarkRenderSystem : public BaseSystem {
 public:
  explicit BenchmarkRenderSystem(rse::Renderer2D* renderer) : BaseSystem(), m_renderer(renderer) {}

  void SetEnabled(bool enabled) { m_enabled = enabled; }
  bool IsEnabled() const { return m_enabled; }

  // Rebuilds the grid. Cheap enough to call from an ImGui slider: one vector of
  // transforms and no other allocation.
  void SetCount(std::size_t count) {
    m_transforms.clear();

    if (count == 0) {
      return;
    }

    const std::size_t columns = static_cast<std::size_t>(std::ceil(std::sqrt(static_cast<double>(count))));
    const std::size_t rows = (count + columns - 1) / columns;

    // The cell that keeps the whole grid inside the default view.
    const float cell = std::min(kWidth / static_cast<float>(columns), kHeight / static_cast<float>(rows));

    const float origin_x = kCenterX - (static_cast<float>(columns) * cell) * 0.5f;
    const float origin_y = kCenterY - (static_cast<float>(rows) * cell) * 0.5f;

    // A sprite's size is a half extent and the transform scales it, so a scale
    // of 0.45 of a cell leaves a tenth of the cell as a gap.
    const float scale = cell * 0.45f;

    m_transforms.reserve(count);

    for (std::size_t index = 0; index < count; ++index) {
      const std::size_t column = index % columns;
      const std::size_t row = index / columns;

      TransformComponent transform;
      transform.position = {origin_x + (static_cast<float>(column) + 0.5f) * cell,
                            origin_y + (static_cast<float>(row) + 0.5f) * cell, 0.0f};
      transform.scale = {scale, scale, 1.0f};
      m_transforms.push_back(transform);
    }
  }

  std::size_t GetCount() const { return m_transforms.size(); }

  void Update(rse::Scene&, const float alpha) override {
    (void)alpha;

    if (!m_enabled || m_renderer == nullptr) {
      return;
    }

    for (TransformComponent& transform : m_transforms) {
      m_renderer->Submit(transform, &m_sprite, kLayer);
    }
  }

 private:
  // Below everything the scene submits, so the benchmark never hides the scene
  // it is being measured next to.
  static constexpr std::int32_t kLayer{-100};

  static constexpr float kWidth{1280.0f};
  static constexpr float kHeight{720.0f};
  static constexpr float kCenterX{640.0f};
  static constexpr float kCenterY{360.0f};

  rse::Renderer2D* m_renderer{nullptr};

  // One untextured quad, shared by every instance: the benchmark is about the
  // number of submissions, not about sampling.
  rse::Sprite m_sprite{glm::vec3(0.0f), glm::vec2(1.0f), rse::MakeColor(64, 64, 64)};

  std::vector<TransformComponent> m_transforms;
  bool m_enabled{false};
};
