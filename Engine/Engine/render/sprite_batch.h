#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

#include "batch_limits.h"

namespace rse {
class MaterialShader;
class Shader;

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
  // The batch's texture unit this vertex samples, or -1 when the quad is
  // untextured and uses its colour alone. It is a float because the attribute
  // is declared float in the shader and interpolated to the fragment stage.
  float slot;
  std::uint32_t color;
};

// One draw call's worth of geometry. The indices are the shared quad pattern
// (see SpriteBatchBuilder::WriteQuadIndices): they are local to the batch and
// start at zero, so a backend passes first_vertex as the base vertex instead of
// storing an index range per batch.
struct SpriteBatch {
  MaterialShader* material{nullptr};
  // The program this batch draws with, resolved from the material when the
  // batch is built; null means the renderer's scene shader. Resolving it once
  // keeps the per-frame draw loop from touching the material.
  Shader* shader{nullptr};
  std::size_t first_vertex{0};
  std::size_t vertex_count{0};
  std::size_t index_count{0};
  // The textures the batch samples, bound to units 0..texture_count-1. A batch
  // of untextured quads has none.
  std::array<std::uint32_t, kMaxTextureSlots> textures{};
  std::size_t texture_count{0};
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
  // Writes every quad's six indices, advanced by four per quad: 0,1,2, 2,3,0.
  // The pattern is the same for every batch, so a backend uploads it once for
  // the largest batch instead of rebuilding it from the frame's vertices.
  // `indices` must have room for quad_count * kIndicesPerQuad entries.
  static void WriteQuadIndices(std::uint16_t* indices, std::size_t quad_count);

  void Reset();

  // How many textures a batch may sample. The renderer lowers this when the
  // context reports fewer units than kMaxTextureSlots; a fixed-function or web
  // backend can lower it further. Clamped to [1, kMaxTextureSlots] and kept
  // across Reset(), because it describes the backend, not the frame.
  void SetMaxTextureSlots(std::size_t slots);
  std::size_t GetMaxTextureSlots() const { return m_max_texture_slots; }

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
  const std::vector<SpriteBatch>& GetBatches() const { return m_batches; }
  const SpriteBatchStats& GetStats() const { return m_stats; }

 private:
  bool IsVisible(const glm::vec3 (&corners)[4]) const;
  glm::mat4 ModelFor(const SpriteDraw& draw) const;

  std::vector<SpriteDraw> m_draws;
  std::vector<SpriteVertex> m_vertices;
  std::vector<SpriteBatch> m_batches;
  SpriteBatchStats m_stats;

  glm::vec2 m_view_min{0.0f};
  glm::vec2 m_view_max{0.0f};
  bool m_has_view_bounds{false};
  std::size_t m_max_texture_slots{kMaxTextureSlots};
  std::uint32_t m_next_sequence{0};
};
}  // namespace rse
