#pragma once

#include <memory>

#include <gtest/gtest.h>

#include <components/texture.h>
#include <core/texture_manager.h>

/*
The manager owns one texture per name and components only borrow it, so the
tests that matter are the ones about identity and lifetime, not about pixels.
The files named here do not exist: a missing image still loads, as a texture
with id 0 that draws white, which is what keeps a scene referring to it
openable.
*/
namespace {

using namespace rse;

class TextureManagerTest : public ::testing::Test {
 protected:
  void TearDown() override { TextureManager::Clean(); }
};

TEST_F(TextureManagerTest, AddTexture) {
  TextureManager::Add(std::make_unique<Texture>("test", "test.png"));

  EXPECT_NE(nullptr, TextureManager::Get("test"));
  EXPECT_EQ(1u, TextureManager::Count());
}

TEST_F(TextureManagerTest, AddTexturePNGFormat) {
  TextureManager::Add(std::make_unique<Texture>("test_png", "test.png"));
  TextureManager::Add(std::make_unique<Texture>("test_jpeg", "test.jpeg"));

  EXPECT_NE(nullptr, TextureManager::Get("test_png"));
  EXPECT_NE(nullptr, TextureManager::Get("test_jpeg"));
  EXPECT_EQ(2u, TextureManager::Count());
}

TEST_F(TextureManagerTest, GetOfAnUnknownNameIsNull) {
  EXPECT_EQ(nullptr, TextureManager::Get("nothing_was_registered"));
}

TEST_F(TextureManagerTest, GetOrLoadLoadsANameOnce) {
  Texture* first = TextureManager::GetOrLoad("shared.png");
  Texture* second = TextureManager::GetOrLoad("shared.png");

  ASSERT_NE(nullptr, first);
  EXPECT_EQ(first, second);
  EXPECT_EQ(first, TextureManager::Get("shared.png"));
  EXPECT_EQ(1u, TextureManager::Count());
}

TEST_F(TextureManagerTest, GetOrLoadKeepsOneTexturePerName) {
  Texture* player = TextureManager::GetOrLoad("shared.png");
  Texture* crate = TextureManager::GetOrLoad("other.png");

  EXPECT_NE(player, crate);
  EXPECT_EQ(2u, TextureManager::Count());
}

TEST_F(TextureManagerTest, AMissingFileLoadsWithNoId) {
  Texture* texture = TextureManager::GetOrLoad("this_file_does_not_exist.png");

  ASSERT_NE(nullptr, texture);
  EXPECT_EQ("this_file_does_not_exist.png", texture->GetName());
  EXPECT_EQ(0u, texture->GetId());
}

TEST_F(TextureManagerTest, AnEmptyNameMeansNoTexture) { EXPECT_EQ(nullptr, TextureManager::GetOrLoad("")); }

TEST_F(TextureManagerTest, CleanTwiceIsSafe) {
  TextureManager::GetOrLoad("shared.png");

  TextureManager::Clean();
  EXPECT_EQ(0u, TextureManager::Count());
  EXPECT_EQ(nullptr, TextureManager::Get("shared.png"));

  TextureManager::Clean();
  EXPECT_EQ(0u, TextureManager::Count());
}
}  // namespace
