#pragma once

#include <gtest/gtest.h>

#include <components/texture.h>
#include <core/texture_manager.h>

/*
TODO I need to change the default behavior for constructing and calling Texture objects currently
it never fails but I should but a proper logic that would crash if something is missing, this is going
to be done after the new ECS comes into place
*/
namespace {

using namespace rse;

TEST(TextureManagerTest, AddTexture) {
  TextureManager::Add(new Texture("test", "test.png"));
  Texture* test_texture = TextureManager::Get("test");
  EXPECT_NE(nullptr, test_texture);
}

TEST(TextureManagerTest, AddTexturePNGFormat) {
  TextureManager::Add(new Texture("test", "test.png"));
  Texture* test_texture = TextureManager::Get("test");
  EXPECT_NE(nullptr, test_texture);
}

TEST(TextureManagerTest, AddTextureJPEGFormat) {
  TextureManager::Add(new Texture("test", "test.jpeg"));
  Texture* test_texture = TextureManager::Get("test");
  EXPECT_NE(nullptr, test_texture);
}
}  // namespace