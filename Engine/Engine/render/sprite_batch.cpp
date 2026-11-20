#include "sprite_batch.h"

#include <algorithm>
#include <cmath>
#include <functional>

#include <glm/gtc/matrix_transform.hpp>

#include <components/material_shader.h>

namespace rse {
namespace {
// Rotation is optional for 2D sprites, and the common case is a quad that only
// needs a translation and a non-uniform scale. The exact zero test keeps the
// fast path predictable: anything authored as a rotation takes the matrix path.
bool IsUnrotated(const SpriteDraw& draw) {
  return draw.rotation.x == 0.0f && draw.rotation.y == 0.0f && draw.rotation.z == 0.0f;
}

// The unit a texture is already bound to in this batch, or -1 when the batch
// does not hold it yet.
int FindTextureSlot(const std::array<std::uint32_t, kMaxTextureSlots>& textures, std::size_t count,
                    std::uint32_t texture) {
  for (std::size_t slot = 0; slot < count; ++slot) {
    if (textures[slot] == texture) return static_cast<int>(slot);
  }
  return -1;
}
}  // namespace

void SpriteBatchBuilder::SetMaxTextureSlots(std::size_t slots) {
  m_max_texture_slots = std::clamp<std::size_t>(slots, 1, kMaxTextureSlots);
}

void SpriteBatchBuilder::Reset() {
  m_draws.clear();
  m_vertices.clear();
  m_batches.clear();
  m_stats = {};
  m_next_sequence = 0;
}

void SpriteBatchBuilder::WriteQuadIndices(std::uint16_t* indices, std::size_t quad_count) {
  std::uint16_t base = 0;

  for (std::size_t quad = 0; quad < quad_count; ++quad) {
    indices[0] = static_cast<std::uint16_t>(base + 0);
    indices[1] = static_cast<std::uint16_t>(base + 1);
    indices[2] = static_cast<std::uint16_t>(base + 2);
    indices[3] = static_cast<std::uint16_t>(base + 2);
    indices[4] = static_cast<std::uint16_t>(base + 3);
    indices[5] = static_cast<std::uint16_t>(base + 0);

    indices += kIndicesPerQuad;
    base = static_cast<std::uint16_t>(base + kVerticesPerQuad);
  }
}

void SpriteBatchBuilder::SetViewBounds(const glm::vec2& min, const glm::vec2& max) {
  m_view_min = min;
  m_view_max = max;
  m_has_view_bounds = true;
}

void SpriteBatchBuilder::ClearViewBounds() { m_has_view_bounds = false; }

void SpriteBatchBuilder::Add(const SpriteDraw& draw) {
  SpriteDraw copy = draw;
  copy.sequence = m_next_sequence++;
  m_draws.push_back(copy);
}

void SpriteBatchBuilder::AddLine(const glm::vec3& start, const glm::vec3& end, float thickness, std::uint32_t color,
                                 std::int32_t layer) {
  const glm::vec2 delta(end.x - start.x, end.y - start.y);
  const float length = glm::length(delta);

  // A point has no direction to orient a quad along, and a zero width covers no
  // pixels. Neither is an error: there is simply nothing to draw.
  if (length <= 0.0f || thickness <= 0.0f) return;

  SpriteDraw draw;
  draw.position = (start + end) * 0.5f;
  draw.rotation = glm::vec3(0.0f, 0.0f, glm::degrees(std::atan2(delta.y, delta.x)));
  // size is a half extent, so half the line's length and half its width.
  draw.size = glm::vec2(length * 0.5f, thickness * 0.5f);
  draw.color = color;
  draw.texture = 0;
  draw.layer = layer;
  draw.depth = draw.position.z;
  Add(draw);
}

void SpriteBatchBuilder::AddRect(const glm::vec3& center, const glm::vec2& half_extent, float rotation_degrees,
                                 std::uint32_t color, std::int32_t layer) {
  if (half_extent.x <= 0.0f || half_extent.y <= 0.0f) return;

  SpriteDraw draw;
  draw.position = center;
  draw.rotation = glm::vec3(0.0f, 0.0f, rotation_degrees);
  draw.size = half_extent;
  draw.color = color;
  draw.texture = 0;
  draw.layer = layer;
  draw.depth = center.z;
  Add(draw);
}

void SpriteBatchBuilder::AddCircle(const glm::vec3& center, float radius, float thickness, std::uint32_t color,
                                   int segments, std::int32_t layer) {
  if (radius <= 0.0f || thickness <= 0.0f) return;

  // Three segments is the fewest that still encloses an area. The upper bound
  // keeps a mistyped segment count from turning one circle into a million
  // quads; past a few hundred segments the outline cannot look smoother.
  const int count = std::clamp(segments, 3, 1024);
  constexpr float kTwoPi = 6.28318530717958647692f;

  glm::vec2 previous(radius, 0.0f);

  for (int i = 1; i <= count; ++i) {
    const float angle = kTwoPi * static_cast<float>(i) / static_cast<float>(count);
    const glm::vec2 next(radius * std::cos(angle), radius * std::sin(angle));

    AddLine(glm::vec3(center.x + previous.x, center.y + previous.y, center.z),
            glm::vec3(center.x + next.x, center.y + next.y, center.z), thickness, color, layer);

    previous = next;
  }
}

glm::mat4 SpriteBatchBuilder::ModelFor(const SpriteDraw& draw) const {
  glm::mat4 model = glm::translate(glm::mat4(1.0f), draw.position);
  model = glm::rotate(model, glm::radians(draw.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
  model = glm::rotate(model, glm::radians(draw.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
  model = glm::rotate(model, glm::radians(draw.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
  model = glm::scale(model, glm::vec3(draw.scale, 1.0f));
  return model;
}

bool SpriteBatchBuilder::IsVisible(const glm::vec3 (&corners)[4]) const {
  if (!m_has_view_bounds) return true;

  glm::vec2 min(corners[0].x, corners[0].y);
  glm::vec2 max = min;
  for (int i = 1; i < 4; ++i) {
    min.x = std::min(min.x, corners[i].x);
    min.y = std::min(min.y, corners[i].y);
    max.x = std::max(max.x, corners[i].x);
    max.y = std::max(max.y, corners[i].y);
  }

  return !(max.x < m_view_min.x || max.y < m_view_min.y || min.x > m_view_max.x || min.y > m_view_max.y);
}

void SpriteBatchBuilder::Build() {
  m_vertices.clear();
  m_batches.clear();
  m_stats = {};
  m_stats.submitted = m_draws.size();

  // Layers are the coarse ordering primitive; depth is the fine one inside a
  // layer. Back-to-front depth keeps the existing alpha-blended sprites working
  // with the depth test still enabled. Material is the batch key, so it comes
  // before the submission sequence when layer and depth are equal. Texture is
  // deliberately not a key any more: a batch's slot table already holds every
  // texture it samples, so ordering by texture would only reorder sprites that
  // overlap at the same depth. Submission order is what an author expects.
  const auto ordered = [](const SpriteDraw& a, const SpriteDraw& b) {
    if (a.layer != b.layer) return a.layer < b.layer;
    if (a.depth != b.depth) return a.depth > b.depth;
    if (a.material != b.material) return std::less<MaterialShader*>{}(a.material, b.material);
    return a.sequence < b.sequence;
  };

  // A frame that arrives already ordered -- one layer submitted by one system,
  // the common case -- would only be shuffled and put back by the sort. Checking
  // is one pass; sorting is n log n.
  if (!std::is_sorted(m_draws.begin(), m_draws.end(), ordered)) {
    std::sort(m_draws.begin(), m_draws.end(), ordered);
  }

  const auto close_batch = [this](const std::array<std::uint32_t, kMaxTextureSlots>& textures,
                                  std::size_t texture_count, MaterialShader* material, std::size_t first_vertex,
                                  std::size_t vertex_count) {
    if (vertex_count == 0) return;

    SpriteBatch batch;
    batch.material = material;
    batch.shader = material == nullptr ? nullptr : material->GetShader();
    batch.first_vertex = first_vertex;
    batch.vertex_count = vertex_count;
    batch.index_count = vertex_count / kVerticesPerQuad * kIndicesPerQuad;
    batch.textures = textures;
    batch.texture_count = texture_count;
    m_batches.push_back(batch);
  };

  std::array<std::uint32_t, kMaxTextureSlots> batch_textures{};
  std::size_t batch_texture_count = 0;
  MaterialShader* batch_material = nullptr;
  std::size_t batch_first_vertex = 0;
  std::size_t batch_vertex_count = 0;
  bool batch_open = false;

  for (const SpriteDraw& draw : m_draws) {
    // Corners in world space, in the same order as the sprite's UVs:
    // lower-left, upper-left, upper-right, lower-right.
    glm::vec3 corners[4];

    if (IsUnrotated(draw)) {
      // The fast path: a centre plus the scaled half extents. No matrix is
      // built, and the quad is already axis aligned so culling uses the same
      // corners the vertices are written from.
      const float half_x = draw.size.x * draw.scale.x;
      const float half_y = draw.size.y * draw.scale.y;
      const glm::vec3& position = draw.position;

      corners[0] = {position.x - half_x, position.y - half_y, position.z};
      corners[1] = {position.x - half_x, position.y + half_y, position.z};
      corners[2] = {position.x + half_x, position.y + half_y, position.z};
      corners[3] = {position.x + half_x, position.y - half_y, position.z};
    } else {
      const glm::mat4 model = ModelFor(draw);
      corners[0] = glm::vec3(model * glm::vec4(-draw.size.x, -draw.size.y, 0.0f, 1.0f));
      corners[1] = glm::vec3(model * glm::vec4(-draw.size.x, draw.size.y, 0.0f, 1.0f));
      corners[2] = glm::vec3(model * glm::vec4(draw.size.x, draw.size.y, 0.0f, 1.0f));
      corners[3] = glm::vec3(model * glm::vec4(draw.size.x, -draw.size.y, 0.0f, 1.0f));
    }

    if (!IsVisible(corners)) {
      ++m_stats.culled;
      continue;
    }

    if (!batch_open || draw.material != batch_material || batch_vertex_count + kVerticesPerQuad > kMaxVertices) {
      if (batch_open) {
        close_batch(batch_textures, batch_texture_count, batch_material, batch_first_vertex, batch_vertex_count);
      }
      batch_textures.fill(0);
      batch_texture_count = 0;
      batch_material = draw.material;
      batch_first_vertex = m_vertices.size();
      batch_vertex_count = 0;
      batch_open = true;
    }

    // The quad joins the batch when its texture is already bound to a unit, or
    // when a unit is still free. An untextured quad needs no unit at all, so it
    // never breaks a batch. A texture that does not fit forces the batch to
    // close and a new one to start with this quad.
    int slot = -1;

    if (draw.texture != 0) {
      slot = FindTextureSlot(batch_textures, batch_texture_count, draw.texture);

      if (slot < 0) {
        if (batch_texture_count == m_max_texture_slots) {
          close_batch(batch_textures, batch_texture_count, batch_material, batch_first_vertex, batch_vertex_count);
          batch_textures.fill(0);
          batch_texture_count = 0;
          batch_first_vertex = m_vertices.size();
          batch_vertex_count = 0;
        }

        slot = static_cast<int>(batch_texture_count);
        batch_textures[batch_texture_count++] = draw.texture;
      }
    }

    const float vertex_slot = slot < 0 ? -1.0f : static_cast<float>(slot);

    // Flip inside the sprite's own UV rectangle, not across the whole texture:
    // an atlas region mirrored around 0.5 would sample a neighbouring image.
    std::array<glm::vec2, 4> uv = draw.uv;

    if (draw.flip_x || draw.flip_y) {
      float min_u = uv[0].x;
      float max_u = uv[0].x;
      float min_v = uv[0].y;
      float max_v = uv[0].y;

      for (const glm::vec2& corner : uv) {
        min_u = std::min(min_u, corner.x);
        max_u = std::max(max_u, corner.x);
        min_v = std::min(min_v, corner.y);
        max_v = std::max(max_v, corner.y);
      }

      for (glm::vec2& corner : uv) {
        if (draw.flip_x) corner.x = min_u + max_u - corner.x;
        if (draw.flip_y) corner.y = min_v + max_v - corner.y;
      }
    }

    for (int i = 0; i < 4; ++i) {
      m_vertices.push_back({corners[i], uv[i], vertex_slot, draw.color});
    }

    batch_vertex_count += kVerticesPerQuad;
  }

  if (batch_open) {
    close_batch(batch_textures, batch_texture_count, batch_material, batch_first_vertex, batch_vertex_count);
  }

  m_stats.batches = m_batches.size();
  m_stats.vertices = m_vertices.size();
  // Every batch's index list is the shared quad pattern, so the frame's index
  // count is the vertex count scaled to six indices per four vertices.
  m_stats.indices = m_vertices.size() / kVerticesPerQuad * kIndicesPerQuad;
}
}  // namespace rse
