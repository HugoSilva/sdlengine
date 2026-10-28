#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

namespace rse {
class MaterialShader;

// A backend-neutral description of one 2D quad. The renderer consumes these
// instead of touching a GPU buffer while the ECS is still submitting sprites:
// that is what lets the batch be sorted, culled and grouped before anything is
// uploaded.
struct SpriteDraw {
  std::array<glm::vec2, 4> uv{
      {glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f)}};
  glm::vec3 position{0.0f};
  glm::vec3 rotation{0.0f};
  glm::vec2 size{1.0f};
  glm::vec2 scale{1.0f};
  std::uint32_t color{0xffffffffu};
  // The backend-neutral texture handle. Zero means "untextured": the quad uses
  // its vertex colour and ignores the sampler.
  std::uint32_t texture{0};
  MaterialShader* material{nullptr};
  bool flip_x{false};
  bool flip_y{false};
  std::int32_t layer{0};
  float depth{0.0f};
  std::uint32_t sequence{0};
};

struct SpriteVertex {
  glm::vec3 position;
  glm::vec2 uv;
  float tid;
  std::uint32_t color;
};

// One draw call's worth of geometry. The indices are local to the batch. A
// backend that uploads each batch separately can draw them as-is; one that keeps
// a shared vertex buffer must pass first_vertex as the base vertex (or copy the
// batch to the start of a scratch buffer first).
struct SpriteBatch {
  std::uint32_t texture{0};
  MaterialShader* material{nullptr};
  std::size_t first_vertex{0};
  std::size_t vertex_count{0};
  std::size_t first_index{0};
  std::size_t index_count{0};
};

struct SpriteBatchStats {
  std::size_t submitted{0};
  std::size_t culled{0};
  std::size_t batches{0};
  std::size_t vertices{0};
  std::size_t indices{0};
};

class SpriteBatchBuilder {
 public:
  static constexpr std::size_t kMaxVerticesPerBatch = 65532;

  void Reset();

  // The world rectangle the 2D camera can see. An empty view (the default)
  // disables culling, which keeps the editor's first frame and headless tests
  // independent of a camera.
  void SetViewBounds(const glm::vec2& min, const glm::vec2& max);
  void ClearViewBounds();

  void Add(const SpriteDraw& draw);

  // Sorts, culls, and builds the vertex/index arrays and batch ranges. Safe to
  // call more than once; each call rebuilds from the submitted draws.
  void Build();

  const std::vector<SpriteVertex>& GetVertices() const { return m_vertices; }
  const std::vector<std::uint16_t>& GetIndices() const { return m_indices; }
  const std::vector<SpriteBatch>& GetBatches() const { return m_batches; }
  const SpriteBatchStats& GetStats() const { return m_stats; }

 private:
  bool IsVisible(const SpriteDraw& draw, const glm::mat4& model) const;
  glm::mat4 ModelFor(const SpriteDraw& draw) const;

  std::vector<SpriteDraw> m_draws;
  std::vector<SpriteVertex> m_vertices;
  std::vector<std::uint16_t> m_indices;
  std::vector<SpriteBatch> m_batches;
  SpriteBatchStats m_stats;

  glm::vec2 m_view_min{0.0f};
  glm::vec2 m_view_max{0.0f};
  bool m_has_view_bounds{false};
  std::uint32_t m_next_sequence{0};
};
}  // namespace rse
