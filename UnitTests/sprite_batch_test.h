#pragma once

#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

#include <components/material_shader.h>
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

TEST(SpriteBatchTest, GroupsTwoTexturesIntoOneBatch) {
  SpriteBatchBuilder builder;
  builder.Add(MakeDraw(2));
  builder.Add(MakeDraw(1));
  builder.Add(MakeDraw(2));
  builder.Build();

  ASSERT_EQ(1u, builder.GetBatches().size());
  EXPECT_EQ(2u, builder.GetBatches()[0].texture_count);
  // The slot table follows submission order: the first texture submitted is
  // unit 0, whichever texture it is.
  EXPECT_EQ(2u, builder.GetBatches()[0].textures[0]);
  EXPECT_EQ(1u, builder.GetBatches()[0].textures[1]);
  EXPECT_EQ(12u, builder.GetVertices().size());
  EXPECT_EQ(18u, builder.GetStats().indices);
  EXPECT_EQ(0u, builder.GetBatches()[0].first_vertex);
  EXPECT_EQ(18u, builder.GetBatches()[0].index_count);
  EXPECT_EQ(3u, builder.GetStats().submitted);
  EXPECT_EQ(0u, builder.GetStats().culled);
  EXPECT_EQ(1u, builder.GetStats().batches);
}

TEST(SpriteBatchTest, SharedQuadPatternIsLocalToABatch) {
  std::vector<std::uint16_t> indices(kIndicesPerQuad * 2);
  SpriteBatchBuilder::WriteQuadIndices(indices.data(), 2);

  const std::vector<std::uint16_t> expected = {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4};
  EXPECT_EQ(expected, indices);
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

TEST(SpriteBatchTest, UnrotatedQuadScalesTheHalfExtents) {
  SpriteBatchBuilder builder;
  SpriteDraw draw = MakeDraw(1);
  draw.position = glm::vec3(2.0f, 3.0f, 0.5f);
  draw.size = glm::vec2(1.0f, 2.0f);
  draw.scale = glm::vec2(2.0f, 3.0f);
  builder.Add(draw);
  builder.Build();

  ASSERT_EQ(4u, builder.GetVertices().size());
  // Half extents are size * scale: (2, 6) around the centre.
  EXPECT_FLOAT_EQ(0.0f, builder.GetVertices()[0].position.x);
  EXPECT_FLOAT_EQ(-3.0f, builder.GetVertices()[0].position.y);
  EXPECT_FLOAT_EQ(0.0f, builder.GetVertices()[1].position.x);
  EXPECT_FLOAT_EQ(9.0f, builder.GetVertices()[1].position.y);
  EXPECT_FLOAT_EQ(4.0f, builder.GetVertices()[2].position.x);
  EXPECT_FLOAT_EQ(9.0f, builder.GetVertices()[2].position.y);
  EXPECT_FLOAT_EQ(4.0f, builder.GetVertices()[3].position.x);
  EXPECT_FLOAT_EQ(-3.0f, builder.GetVertices()[3].position.y);
  for (const SpriteVertex& vertex : builder.GetVertices()) {
    EXPECT_FLOAT_EQ(0.5f, vertex.position.z);
  }
}

TEST(SpriteBatchTest, RotatedQuadTurnsOntoTheOtherAxis) {
  SpriteBatchBuilder builder;
  SpriteDraw draw = MakeDraw(1);
  draw.size = glm::vec2(1.0f);
  draw.rotation.z = 90.0f;
  builder.Add(draw);
  builder.Build();

  ASSERT_EQ(4u, builder.GetVertices().size());
  // A 90 degree turn about z maps (x, y) to (-y, x), so the lower-left corner
  // lands where the lower-right one was. cos(90) is only approximately zero, so
  // compare with a tolerance rather than exactly.
  EXPECT_NEAR(1.0f, builder.GetVertices()[0].position.x, 1e-4f);
  EXPECT_NEAR(-1.0f, builder.GetVertices()[0].position.y, 1e-4f);
  EXPECT_NEAR(-1.0f, builder.GetVertices()[1].position.x, 1e-4f);
  EXPECT_NEAR(-1.0f, builder.GetVertices()[1].position.y, 1e-4f);
  EXPECT_NEAR(-1.0f, builder.GetVertices()[2].position.x, 1e-4f);
  EXPECT_NEAR(1.0f, builder.GetVertices()[2].position.y, 1e-4f);
  EXPECT_NEAR(1.0f, builder.GetVertices()[3].position.x, 1e-4f);
  EXPECT_NEAR(1.0f, builder.GetVertices()[3].position.y, 1e-4f);
}

TEST(SpriteBatchTest, CullsByTheRotatedExtent) {
  SpriteBatchBuilder builder;
  builder.SetViewBounds(glm::vec2(-10.0f), glm::vec2(10.0f));

  // A long, thin quad whose centre is outside the view. Unrotated it is far to
  // the right and invisible; turned 90 degrees it becomes 24 units wide and
  // reaches into the view, so culling has to use the transformed corners rather
  // than the sprite's centre.
  SpriteDraw unrotated = MakeDraw(1, 15.0f);
  unrotated.size = glm::vec2(1.0f, 12.0f);
  builder.Add(unrotated);

  SpriteDraw rotated = MakeDraw(1, 15.0f);
  rotated.size = glm::vec2(1.0f, 12.0f);
  rotated.rotation.z = 90.0f;
  builder.Add(rotated);

  builder.Build();

  EXPECT_EQ(1u, builder.GetStats().culled);
  ASSERT_EQ(4u, builder.GetVertices().size());
  EXPECT_NEAR(27.0f, builder.GetVertices()[0].position.x, 1e-4f);
  EXPECT_NEAR(-1.0f, builder.GetVertices()[0].position.y, 1e-4f);
  EXPECT_NEAR(3.0f, builder.GetVertices()[1].position.x, 1e-4f);
}

TEST(SpriteBatchTest, SplitsBatchesAtTheSixteenBitVertexLimit) {
  constexpr std::size_t quads_per_batch = kMaxQuads;
  SpriteBatchBuilder builder;
  for (std::size_t i = 0; i < quads_per_batch + 1; ++i) {
    builder.Add(MakeDraw(1));
  }
  builder.Build();

  ASSERT_EQ(2u, builder.GetBatches().size());
  EXPECT_EQ(kMaxVertices, builder.GetBatches()[0].vertex_count);
  EXPECT_EQ(4u, builder.GetBatches()[1].vertex_count);
  EXPECT_EQ(quads_per_batch * kIndicesPerQuad, builder.GetBatches()[0].index_count);
  EXPECT_EQ(kIndicesPerQuad, builder.GetBatches()[1].index_count);
}

TEST(SpriteBatchTest, UntexturedDrawUsesVertexColourPath) {
  SpriteBatchBuilder builder;
  SpriteDraw draw = MakeDraw(0);
  draw.uv = {{glm::vec2(0.25f, 0.25f), glm::vec2(0.25f, 0.25f), glm::vec2(0.25f, 0.25f), glm::vec2(0.25f, 0.25f)}};
  draw.color = 0xff00ff00u;
  builder.Add(draw);
  builder.Build();

  ASSERT_EQ(4u, builder.GetVertices().size());
  EXPECT_FLOAT_EQ(-1.0f, builder.GetVertices()[0].slot);
  EXPECT_EQ(0u, builder.GetBatches()[0].texture_count);
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
  MaterialShader first{"first", "First", "", nullptr};
  MaterialShader second{"second", "Second", "", nullptr};

  SpriteBatchBuilder builder;
  SpriteDraw first_draw = MakeDraw(7);
  first_draw.material = &first;
  SpriteDraw second_draw = MakeDraw(7);
  second_draw.material = &second;

  builder.Add(first_draw);
  builder.Add(second_draw);
  builder.Build();

  ASSERT_EQ(2u, builder.GetBatches().size());
  EXPECT_NE(builder.GetBatches()[0].material, builder.GetBatches()[1].material);
  EXPECT_EQ(7u, builder.GetBatches()[0].textures[0]);
  EXPECT_EQ(7u, builder.GetBatches()[1].textures[0]);
  EXPECT_EQ(0u, builder.GetBatches()[0].first_vertex);
  EXPECT_EQ(4u, builder.GetBatches()[1].first_vertex);
  // A material that did not compile resolves to no program, which is the
  // renderer's signal to fall back to the scene shader.
  EXPECT_EQ(nullptr, builder.GetBatches()[0].shader);
  EXPECT_EQ(nullptr, builder.GetBatches()[1].shader);
}

TEST(SpriteBatchTest, NoMaterialResolvesToTheSceneShader) {
  SpriteBatchBuilder builder;
  builder.Add(MakeDraw(7));
  builder.Build();

  ASSERT_EQ(1u, builder.GetBatches().size());
  EXPECT_EQ(nullptr, builder.GetBatches()[0].material);
  EXPECT_EQ(nullptr, builder.GetBatches()[0].shader);
}

TEST(SpriteBatchTest, TexturedDrawSamplesSlotZero) {
  SpriteBatchBuilder builder;
  builder.Add(MakeDraw(7));
  builder.Build();

  ASSERT_EQ(4u, builder.GetVertices().size());
  EXPECT_FLOAT_EQ(0.0f, builder.GetVertices()[0].slot);
  ASSERT_EQ(1u, builder.GetBatches().size());
  EXPECT_EQ(1u, builder.GetBatches()[0].texture_count);
  EXPECT_EQ(7u, builder.GetBatches()[0].textures[0]);
}

TEST(SpriteBatchTest, BatchesSixteenTexturesInOneDrawCall) {
  SpriteBatchBuilder builder;
  for (std::uint32_t texture = 1; texture <= kMaxTextureSlots; ++texture) {
    builder.Add(MakeDraw(texture));
  }
  builder.Build();

  ASSERT_EQ(1u, builder.GetBatches().size());
  EXPECT_EQ(kMaxTextureSlots, builder.GetBatches()[0].texture_count);

  // The draw list is sorted by texture, so texture 1 lands in unit 0 and each
  // following one in the next unit.
  for (std::size_t slot = 0; slot < kMaxTextureSlots; ++slot) {
    EXPECT_EQ(static_cast<std::uint32_t>(slot + 1), builder.GetBatches()[0].textures[slot]);
    EXPECT_FLOAT_EQ(static_cast<float>(slot), builder.GetVertices()[slot * kVerticesPerQuad].slot);
  }
}

TEST(SpriteBatchTest, OpensASecondBatchForTheSeventeenthTexture) {
  SpriteBatchBuilder builder;
  for (std::uint32_t texture = 1; texture <= kMaxTextureSlots + 1; ++texture) {
    builder.Add(MakeDraw(texture));
  }
  builder.Build();

  ASSERT_EQ(2u, builder.GetBatches().size());
  EXPECT_EQ(kMaxTextureSlots, builder.GetBatches()[0].texture_count);
  EXPECT_EQ(1u, builder.GetBatches()[1].texture_count);
  EXPECT_EQ(static_cast<std::uint32_t>(kMaxTextureSlots + 1), builder.GetBatches()[1].textures[0]);
  EXPECT_EQ(2u, builder.GetStats().batches);
}

TEST(SpriteBatchTest, ReusesOneSlotForARepeatedTexture) {
  SpriteBatchBuilder builder;
  for (int i = 0; i < 4; ++i) {
    builder.Add(MakeDraw(3));
    builder.Add(MakeDraw(9));
  }
  builder.Build();

  ASSERT_EQ(1u, builder.GetBatches().size());
  EXPECT_EQ(2u, builder.GetBatches()[0].texture_count);
  EXPECT_EQ(3u, builder.GetBatches()[0].textures[0]);
  EXPECT_EQ(9u, builder.GetBatches()[0].textures[1]);
  // The first quad of each texture names its own unit; repeats reuse it.
  EXPECT_FLOAT_EQ(0.0f, builder.GetVertices()[0].slot);
  EXPECT_FLOAT_EQ(1.0f, builder.GetVertices()[kVerticesPerQuad].slot);
}

TEST(SpriteBatchTest, UntexturedQuadsJoinAFullBatch) {
  SpriteBatchBuilder builder;
  for (std::uint32_t texture = 1; texture <= kMaxTextureSlots; ++texture) {
    builder.Add(MakeDraw(texture));
  }
  builder.Add(MakeDraw(0));
  builder.Build();

  ASSERT_EQ(1u, builder.GetBatches().size());
  EXPECT_EQ(kMaxTextureSlots, builder.GetBatches()[0].texture_count);
  // An untextured quad consumes no unit, so it never needs a batch of its own.
  // It was submitted last, so it is the last vertex of the batch.
  EXPECT_FLOAT_EQ(-1.0f, builder.GetVertices().back().slot);
}

TEST(SpriteBatchTest, KeepsSubmissionOrderForEqualDepthSprites) {
  SpriteBatchBuilder builder;
  builder.Add(MakeDraw(9, 1.0f));
  builder.Add(MakeDraw(3, 2.0f));
  builder.Add(MakeDraw(9, 3.0f));
  builder.Build();

  ASSERT_EQ(1u, builder.GetBatches().size());
  EXPECT_EQ(2u, builder.GetBatches()[0].texture_count);
  // Texture is no longer a sort key, so sprites at the same layer and depth
  // draw in the order they were submitted: an author's alpha overlaps survive.
  EXPECT_FLOAT_EQ(0.0f, builder.GetVertices()[0].position.x);
  EXPECT_FLOAT_EQ(1.0f, builder.GetVertices()[4].position.x);
  EXPECT_FLOAT_EQ(2.0f, builder.GetVertices()[8].position.x);
  // The repeated texture reuses the unit it was given the first time.
  EXPECT_FLOAT_EQ(0.0f, builder.GetVertices()[0].slot);
  EXPECT_FLOAT_EQ(1.0f, builder.GetVertices()[4].slot);
  EXPECT_FLOAT_EQ(0.0f, builder.GetVertices()[8].slot);
}

TEST(SpriteBatchTest, SetMaxTextureSlotsLimitsTheBatchTextures) {
  SpriteBatchBuilder builder;
  builder.SetMaxTextureSlots(4);
  EXPECT_EQ(4u, builder.GetMaxTextureSlots());

  for (std::uint32_t texture = 1; texture <= 8; ++texture) {
    builder.Add(MakeDraw(texture));
  }
  builder.Build();

  ASSERT_EQ(2u, builder.GetBatches().size());
  EXPECT_EQ(4u, builder.GetBatches()[0].texture_count);
  EXPECT_EQ(4u, builder.GetBatches()[1].texture_count);
  EXPECT_EQ(5u, builder.GetBatches()[1].textures[0]);
}

TEST(SpriteBatchTest, SetMaxTextureSlotsStaysWithinTheHeaderLimit) {
  SpriteBatchBuilder builder;
  builder.SetMaxTextureSlots(1000);
  EXPECT_EQ(kMaxTextureSlots, builder.GetMaxTextureSlots());
  builder.SetMaxTextureSlots(0);
  EXPECT_EQ(1u, builder.GetMaxTextureSlots());
}

}  // namespace
