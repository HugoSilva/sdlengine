#pragma once

#include <cstddef>
#include <vector>

#include <gtest/gtest.h>

#include <render/texture_atlas.h>

namespace {

using namespace rse;

bool Overlaps(const AtlasRegion& a, const AtlasRegion& b) {
  return a.x < b.x + b.width && b.x < a.x + a.width && a.y < b.y + b.height && b.y < a.y + a.height;
}

TEST(TextureAtlasTest, PacksShelfAndGrowsPage) {
  AtlasPacker packer(64, 1);
  AtlasRegion a;
  AtlasRegion b;
  AtlasRegion c;
  AtlasRegion d;

  EXPECT_TRUE(packer.Add(15, 15, &a));
  EXPECT_TRUE(packer.Add(15, 15, &b));
  EXPECT_TRUE(packer.Add(15, 15, &c));
  EXPECT_TRUE(packer.Add(15, 15, &d));

  EXPECT_EQ(0, a.x);
  EXPECT_EQ(16, b.x);
  EXPECT_EQ(32, c.x);
  EXPECT_EQ(0, d.y);
  EXPECT_EQ(48, d.x);
  EXPECT_EQ(64, packer.GetPageWidth());
  EXPECT_EQ(16, packer.GetPageHeight());
}

TEST(TextureAtlasTest, StartsANewShelfWhenWidthRunsOut) {
  AtlasPacker packer(32, 1);
  AtlasRegion first;
  AtlasRegion second;
  AtlasRegion third;

  EXPECT_TRUE(packer.Add(20, 10, &first));
  EXPECT_TRUE(packer.Add(20, 10, &second));
  EXPECT_TRUE(packer.Add(20, 10, &third));

  EXPECT_EQ(0, first.x);
  EXPECT_EQ(0, first.y);
  EXPECT_EQ(0, second.x);
  EXPECT_EQ(11, second.y);
  EXPECT_EQ(0, third.x);
  EXPECT_EQ(22, third.y);
  EXPECT_EQ(32, packer.GetPageWidth());
  EXPECT_EQ(32, packer.GetPageHeight());
}

TEST(TextureAtlasTest, RejectsOversizedRectangles) {
  AtlasPacker packer(16, 1);
  AtlasRegion region;
  EXPECT_FALSE(packer.Add(17, 1, &region));
  EXPECT_FALSE(packer.Add(1, 17, &region));
  EXPECT_FALSE(packer.Add(0, 1, &region));
  EXPECT_TRUE(packer.IsEmpty());
}

TEST(TextureAtlasTest, FailedAddLeavesPackerUnchanged) {
  AtlasPacker packer(16, 1);
  AtlasRegion first;
  AtlasRegion too_tall;
  EXPECT_TRUE(packer.Add(16, 16, &first));
  EXPECT_FALSE(packer.Add(1, 1, &too_tall));
  EXPECT_EQ(16, packer.GetUsedWidth());
  EXPECT_EQ(16, packer.GetUsedHeight());
}

TEST(TextureAtlasTest, RegionsDoNotOverlap) {
  AtlasPacker packer(64, 1);
  std::vector<AtlasRegion> regions;
  for (int i = 0; i < 20; ++i) {
    AtlasRegion region;
    ASSERT_TRUE(packer.Add(10, 10, &region));
    regions.push_back(region);
  }

  for (std::size_t i = 0; i < regions.size(); ++i) {
    for (std::size_t j = i + 1; j < regions.size(); ++j) {
      EXPECT_FALSE(Overlaps(regions[i], regions[j])) << "regions " << i << " and " << j << " overlap";
    }
  }

  EXPECT_LE(packer.GetPageWidth(), 64);
  EXPECT_LE(packer.GetPageHeight(), 64);
}

TEST(TextureAtlasTest, EmptyPackerHasOnePixelPage) {
  AtlasPacker packer(64);
  EXPECT_TRUE(packer.IsEmpty());
  EXPECT_EQ(1, packer.GetPageWidth());
  EXPECT_EQ(1, packer.GetPageHeight());
}

TEST(TextureAtlasTest, PageDimensionsArePowersOfTwo) {
  EXPECT_EQ(1, AtlasPacker::NextPowerOfTwo(0));
  EXPECT_EQ(1, AtlasPacker::NextPowerOfTwo(1));
  EXPECT_EQ(2, AtlasPacker::NextPowerOfTwo(2));
  EXPECT_EQ(4, AtlasPacker::NextPowerOfTwo(3));
  EXPECT_EQ(64, AtlasPacker::NextPowerOfTwo(33));
}

TEST(TextureAtlasTest, OpensANewPageWhenEveryPageIsFull) {
  AtlasBuilder builder(32, 0);

  const AtlasPlacement first = builder.Add(32, 32);
  ASSERT_TRUE(first.packed);
  EXPECT_EQ(0u, first.page);

  const AtlasPlacement second = builder.Add(32, 32);
  ASSERT_TRUE(second.packed);
  EXPECT_EQ(1u, second.page);
  EXPECT_EQ(0, second.region.x);
  EXPECT_EQ(0, second.region.y);

  EXPECT_EQ(2u, builder.GetPageCount());
  EXPECT_EQ(32, builder.GetPageWidth(0));
  EXPECT_EQ(32, builder.GetPageWidth(1));
}

TEST(TextureAtlasTest, UsesAnEarlierPageThatStillHasRoom) {
  AtlasBuilder builder(32, 0);

  // A small image leaves half of the first shelf empty; a full-page image then
  // opens a second page; the next small one goes back to the first.
  const AtlasPlacement tiny = builder.Add(16, 16);
  ASSERT_TRUE(tiny.packed);
  EXPECT_EQ(0u, tiny.page);

  const AtlasPlacement page_break = builder.Add(32, 32);
  ASSERT_TRUE(page_break.packed);
  EXPECT_EQ(1u, page_break.page);

  const AtlasPlacement another = builder.Add(16, 16);
  ASSERT_TRUE(another.packed);
  EXPECT_EQ(0u, another.page);
  EXPECT_EQ(16, another.region.x);
  EXPECT_EQ(0, another.region.y);
}

TEST(TextureAtlasTest, AnImageLargerThanAPageIsLeftUnpacked) {
  AtlasBuilder builder(32, 1);

  EXPECT_FALSE(builder.Add(33, 8).packed);
  EXPECT_FALSE(builder.Add(8, 33).packed);
  // A page the image could never fit does not get created.
  EXPECT_EQ(0u, builder.GetPageCount());

  // The rest of the project still packs.
  const AtlasPlacement packed = builder.Add(8, 8);
  ASSERT_TRUE(packed.packed);
  EXPECT_EQ(0u, packed.page);
  EXPECT_EQ(1u, builder.GetPageCount());
}

TEST(TextureAtlasTest, EachPageGrowsToItsOwnContents) {
  AtlasBuilder builder(32, 0);

  ASSERT_TRUE(builder.Add(32, 8).packed);
  // Eight wide and thirty-two tall cannot join the first page's shelf, and the
  // next shelf would overflow, so it starts page 1.
  const AtlasPlacement tall = builder.Add(8, 32);
  ASSERT_TRUE(tall.packed);
  EXPECT_EQ(1u, tall.page);

  EXPECT_EQ(32, builder.GetPageWidth(0));
  EXPECT_EQ(8, builder.GetPageHeight(0));
  EXPECT_EQ(8, builder.GetPageWidth(1));
  EXPECT_EQ(32, builder.GetPageHeight(1));
}

}  // namespace
