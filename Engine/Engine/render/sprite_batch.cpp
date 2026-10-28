#include "sprite_batch.h"

#include <algorithm>
#include <cmath>
#include <functional>

#include <glm/gtc/matrix_transform.hpp>

namespace rse {
void SpriteBatchBuilder::Reset() {
  m_draws.clear();
  m_vertices.clear();
  m_indices.clear();
  m_batches.clear();
  m_stats = {};
  m_next_sequence = 0;
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

glm::mat4 SpriteBatchBuilder::ModelFor(const SpriteDraw& draw) const {
  glm::mat4 model = glm::translate(glm::mat4(1.0f), draw.position);
  model = glm::rotate(model, glm::radians(draw.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
  model = glm::rotate(model, glm::radians(draw.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
  model = glm::rotate(model, glm::radians(draw.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
  model = glm::scale(model, glm::vec3(draw.scale, 1.0f));
  return model;
}

bool SpriteBatchBuilder::IsVisible(const SpriteDraw& draw, const glm::mat4& model) const {
  if (!m_has_view_bounds) return true;

  const glm::vec4 corners[4] = {
      model * glm::vec4(-draw.size.x, -draw.size.y, 0.0f, 1.0f),
      model * glm::vec4(-draw.size.x, draw.size.y, 0.0f, 1.0f),
      model * glm::vec4(draw.size.x, draw.size.y, 0.0f, 1.0f),
      model * glm::vec4(draw.size.x, -draw.size.y, 0.0f, 1.0f),
  };

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
  m_indices.clear();
  m_batches.clear();
  m_stats = {};
  m_stats.submitted = m_draws.size();

  // Layers are the coarse ordering primitive; depth is the fine one inside a
  // layer. Back-to-front depth keeps the existing alpha-blended sprites working
  // with the depth test still enabled. Material and texture are the batching
  // keys, so they come before the submission sequence only when layer and depth
  // are equal.
  std::sort(m_draws.begin(), m_draws.end(), [](const SpriteDraw& a, const SpriteDraw& b) {
    if (a.layer != b.layer) return a.layer < b.layer;
    if (a.depth != b.depth) return a.depth > b.depth;
    if (a.material != b.material) return std::less<MaterialShader*>{}(a.material, b.material);
    if (a.texture != b.texture) return a.texture < b.texture;
    return a.sequence < b.sequence;
  });

  const auto close_batch = [this](std::uint32_t texture, MaterialShader* material, std::size_t first_vertex,
                                  std::size_t vertex_count, std::size_t first_index, std::size_t index_count) {
    if (vertex_count == 0) return;
    m_batches.push_back({texture, material, first_vertex, vertex_count, first_index, index_count});
  };

  std::uint32_t batch_texture = 0;
  MaterialShader* batch_material = nullptr;
  std::size_t batch_first_vertex = 0;
  std::size_t batch_vertex_count = 0;
  std::size_t batch_first_index = 0;
  std::size_t batch_index_count = 0;
  bool batch_open = false;

  for (const SpriteDraw& draw : m_draws) {
    const glm::mat4 model = ModelFor(draw);
    if (!IsVisible(draw, model)) {
      ++m_stats.culled;
      continue;
    }

    if (!batch_open || draw.texture != batch_texture || draw.material != batch_material ||
        batch_vertex_count + 4 > SpriteBatchBuilder::kMaxVerticesPerBatch) {
      if (batch_open) {
        close_batch(batch_texture, batch_material, batch_first_vertex, batch_vertex_count, batch_first_index,
                    batch_index_count);
      }
      batch_texture = draw.texture;
      batch_material = draw.material;
      batch_first_vertex = m_vertices.size();
      batch_vertex_count = 0;
      batch_first_index = m_indices.size();
      batch_index_count = 0;
      batch_open = true;
    }

    const std::uint16_t base = static_cast<std::uint16_t>(batch_vertex_count);
    const float tid = draw.texture == 0 ? 0.0f : 1.0f;

    const glm::vec3 positions[4] = {
        glm::vec3(model * glm::vec4(-draw.size.x, -draw.size.y, 0.0f, 1.0f)),
        glm::vec3(model * glm::vec4(-draw.size.x, draw.size.y, 0.0f, 1.0f)),
        glm::vec3(model * glm::vec4(draw.size.x, draw.size.y, 0.0f, 1.0f)),
        glm::vec3(model * glm::vec4(draw.size.x, -draw.size.y, 0.0f, 1.0f)),
    };

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
      m_vertices.push_back({positions[i], uv[i], tid, draw.color});
    }

    m_indices.push_back(static_cast<std::uint16_t>(base + 0));
    m_indices.push_back(static_cast<std::uint16_t>(base + 1));
    m_indices.push_back(static_cast<std::uint16_t>(base + 2));
    m_indices.push_back(static_cast<std::uint16_t>(base + 2));
    m_indices.push_back(static_cast<std::uint16_t>(base + 3));
    m_indices.push_back(static_cast<std::uint16_t>(base + 0));

    batch_vertex_count += 4;
    batch_index_count += 6;
  }

  if (batch_open) {
    close_batch(batch_texture, batch_material, batch_first_vertex, batch_vertex_count, batch_first_index,
                batch_index_count);
  }

  m_stats.batches = m_batches.size();
  m_stats.vertices = m_vertices.size();
  m_stats.indices = m_indices.size();
}
}  // namespace rse
