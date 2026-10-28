#pragma once

#include <cstdint>

#include <gtest/gtest.h>

#include <render/sprite_batch.h>

namespace {

using namespace rse;

SpriteDraw MakeDraw(std::uint32_t texture, float x = 0.0f, float z = 0.0f, std::int32_t layer = 0) {
  SpriteDraw draw;
  draw.texture = texture;
  draw.position = glm::vec3(x, 0.0f, z);
  draw.depth = z;
  draw.layer = layer;
  draw.size = glm::vec2(1.0f);
  return draw;
}

TEST(SpriteBatchTest, GroupsDrawsByTexture) {
  SpriteBatchBuilder builder;
  builder.Add(MakeDraw(2));
  builder.Add(MakeDraw(1));
  builder.Add(MakeDraw(2));
  builder.Build();

  ASSERT_EQ(2u, builder.GetBatches().size());
  EXPECT_EQ(1u, builder.GetBatches()[0].texture);
  EXPECT_EQ(2u, builder.GetBatches()[1].texture);
  EXPECT_EQ(12u, builder.GetVertices().size());
  EXPECT_EQ(18u, builder.GetIndices().size());
  // The second batch starts at the fifth vertex, so a shared vertex buffer has
  // to draw it with first_vertex as the base vertex.
  EXPECT_EQ(0u, builder.GetBatches()[0].first_vertex);
  EXPECT_EQ(4u, builder.GetBatches()[1].first_vertex);
  EXPECT_EQ(0u, builder.GetBatches()[0].first_index);
  EXPECT_EQ(6u, builder.GetBatches()[1].first_index);
  EXPECT_EQ(3u, builder.GetStats().submitted);
  EXPECT_EQ(0u, builder.GetStats().culled);
  EXPECT_EQ(2u, builder.GetStats().batches);
}

TEST(SpriteBatchTest, IndicesAreLocalToEachBatch) {
  SpriteBatchBuilder builder;
  builder.Add(MakeDraw(2));
  builder.Add(MakeDraw(1));
  builder.Build();

  ASSERT_EQ(2u, builder.GetBatches().size());
  for (const SpriteBatch& batch : builder.GetBatches()) {
    EXPECT_EQ(4u, batch.vertex_count);
    EXPECT_EQ(6u, batch.index_count);
    EXPECT_EQ(0u, builder.GetIndices()[batch.first_index]);
    EXPECT_EQ(0u, builder.GetIndices()[batch.first_index + 5]);
    EXPECT_EQ(3u, builder.GetIndices()[batch.first_index + 4]);
  }
}

TEST(SpriteBatchTest, LayersAndDepthOrderGeometry) {
  SpriteBatchBuilder builder;
  builder.Add(MakeDraw(1, 2.0f, 0.0f, 0));
  builder.Add(MakeDraw(1, 1.0f, 0.0f, 1));
  builder.Add(MakeDraw(1, 0.0f, -1.0f, 1));
  builder.Build();

  // The same texture stays in one batch even across layers, so inspect the
  // sorted vertex order instead of batch boundaries.
  ASSERT_EQ(1u, builder.GetBatches().size());
  // Each quad's first vertex is its lower-left corner, one unit left of its
  // centre. Layer 0 is first even though the layer-1 sprites were submitted
  // later.
  EXPECT_FLOAT_EQ(1.0f, builder.GetVertices()[0].position.x);

  // Within layer 1, larger depth is farther away, so depth 0 is submitted
  // before depth -1 (back to front).
  EXPECT_FLOAT_EQ(0.0f, builder.GetVertices()[4].position.x);
  EXPECT_FLOAT_EQ(-1.0f, builder.GetVertices()[8].position.x);
}

TEST(SpriteBatchTest, CullsOutsideViewBounds) {
  SpriteBatchBuilder builder;
  builder.SetViewBounds(glm::vec2(-10.0f), glm::vec2(10.0f));
  builder.Add(MakeDraw(1, 0.0f));
  builder.Add(MakeDraw(1, 100.0f));
  builder.Build();

  EXPECT_EQ(2u, builder.GetStats().submitted);
  EXPECT_EQ(1u, builder.GetStats().culled);
  EXPECT_EQ(4u, builder.GetVertices().size());
  EXPECT_EQ(1u, builder.GetBatches().size());
}

TEST(SpriteBatchTest, CullingKeepsSpritesThatTouchTheEdge) {
  SpriteBatchBuilder builder;
  builder.SetViewBounds(glm::vec2(-10.0f), glm::vec2(10.0f));
  builder.Add(MakeDraw(1, 10.5f));
  builder.Build();

  EXPECT_EQ(0u, builder.GetStats().culled);
  EXPECT_EQ(4u, builder.GetVertices().size());
}

TEST(SpriteBatchTest, SplitsBatchesAtTheSixteenBitVertexLimit) {
  constexpr std::size_t quads_per_batch = SpriteBatchBuilder::kMaxVerticesPerBatch / 4;
  SpriteBatchBuilder builder;
  for (std::size_t i = 0; i < quads_per_batch + 1; ++i) {
    builder.Add(MakeDraw(1));
  }
  builder.Build();

  ASSERT_EQ(2u, builder.GetBatches().size());
  EXPECT_EQ(SpriteBatchBuilder::kMaxVerticesPerBatch, builder.GetBatches()[0].vertex_count);
  EXPECT_EQ(4u, builder.GetBatches()[1].vertex_count);
  for (std::uint16_t index : builder.GetIndices()) {
    EXPECT_LT(index, builder.GetVertices().size());
  }
}

TEST(SpriteBatchTest, UntexturedDrawUsesVertexColourPath) {
  SpriteBatchBuilder builder;
  SpriteDraw draw = MakeDraw(0);
  draw.uv = {{glm::vec2(0.25f, 0.25f), glm::vec2(0.25f, 0.25f), glm::vec2(0.25f, 0.25f), glm::vec2(0.25f, 0.25f)}};
  draw.color = 0xff00ff00u;
  builder.Add(draw);
  builder.Build();

  ASSERT_EQ(4u, builder.GetVertices().size());
  EXPECT_FLOAT_EQ(0.0f, builder.GetVertices()[0].tid);
  EXPECT_FLOAT_EQ(0.25f, builder.GetVertices()[0].uv.x);
  EXPECT_EQ(0xff00ff00u, builder.GetVertices()[0].color);
}

TEST(SpriteBatchTest, FlipMirrorsUvInsideItsOwnRectangle) {
  SpriteBatchBuilder builder;

  SpriteDraw draw = MakeDraw(1);
  draw.uv = {{glm::vec2(0.25f, 0.75f), glm::vec2(0.25f, 0.5f), glm::vec2(0.5f, 0.5f), glm::vec2(0.5f, 0.75f)}};
  draw.flip_x = true;
  builder.Add(draw);
  builder.Build();

  ASSERT_EQ(4u, builder.GetVertices().size());
  EXPECT_FLOAT_EQ(0.5f, builder.GetVertices()[0].uv.x);
  EXPECT_FLOAT_EQ(0.5f, builder.GetVertices()[1].uv.x);
  EXPECT_FLOAT_EQ(0.25f, builder.GetVertices()[2].uv.x);
  EXPECT_FLOAT_EQ(0.25f, builder.GetVertices()[3].uv.x);
  // Y is untouched, and the atlas rectangle did not grow into a neighbour.
  EXPECT_FLOAT_EQ(0.75f, builder.GetVertices()[0].uv.y);
  EXPECT_FLOAT_EQ(0.5f, builder.GetVertices()[1].uv.y);
}

TEST(SpriteBatchTest, FlipMirrorsBothAxes) {
  SpriteBatchBuilder builder;

  SpriteDraw draw = MakeDraw(1);
  draw.uv = {{glm::vec2(0.0f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f), glm::vec2(1.0f, 1.0f)}};
  draw.flip_x = true;
  draw.flip_y = true;
  builder.Add(draw);
  builder.Build();

  ASSERT_EQ(4u, builder.GetVertices().size());
  EXPECT_FLOAT_EQ(1.0f, builder.GetVertices()[0].uv.x);
  EXPECT_FLOAT_EQ(0.0f, builder.GetVertices()[0].uv.y);
  EXPECT_FLOAT_EQ(0.0f, builder.GetVertices()[3].uv.x);
  EXPECT_FLOAT_EQ(0.0f, builder.GetVertices()[3].uv.y);
}

TEST(SpriteBatchTest, DifferentMaterialsSplitBatchesEvenWithOneTexture) {
  int first_material = 0;
  int second_material = 0;

  SpriteBatchBuilder builder;
  SpriteDraw first = MakeDraw(7);
  first.material = reinterpret_cast<MaterialShader*>(&first_material);
  SpriteDraw second = MakeDraw(7);
  second.material = reinterpret_cast<MaterialShader*>(&second_material);

  builder.Add(first);
  builder.Add(second);
  builder.Build();

  ASSERT_EQ(2u, builder.GetBatches().size());
  EXPECT_NE(builder.GetBatches()[0].material, builder.GetBatches()[1].material);
  EXPECT_EQ(7u, builder.GetBatches()[0].texture);
  EXPECT_EQ(7u, builder.GetBatches()[1].texture);
  EXPECT_EQ(0u, builder.GetBatches()[0].first_index);
  EXPECT_EQ(6u, builder.GetBatches()[1].first_index);
}

TEST(SpriteBatchTest, TexturedDrawSetsTextureFlag) {
  SpriteBatchBuilder builder;
  builder.Add(MakeDraw(7));
  builder.Build();

  ASSERT_EQ(4u, builder.GetVertices().size());
  EXPECT_FLOAT_EQ(1.0f, builder.GetVertices()[0].tid);
}

}  // namespace
