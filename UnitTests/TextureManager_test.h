#pragma once

#include <gtest/gtest.h>

#include <Components/Texture.hpp>
#include <Core/TextureManager.hpp>

/*
TODO I need to change the default behavior for constructing and calling Texture objects currently
it never fails but I should but a proper logic that would crash if something is missing, this is going
to be done after the new ECS comes into place
*/
namespace {

using namespace rse;

TEST(TextureManager, AddTexture) {
  TextureManager::Add(new Texture("test", "test.png"));
  Texture* test_texture = TextureManager::Get("test");
  EXPECT_NE(nullptr, test_texture);
}

TEST(TextureManager, AddTexturePNGFormat) {
  TextureManager::Add(new Texture("test", "test.png"));
  Texture* test_texture = TextureManager::Get("test");
  EXPECT_NE(nullptr, test_texture);
}

TEST(TextureManager, AddTextureJPEGFormat) {
  TextureManager::Add(new Texture("test", "test.jpeg"));
  Texture* test_texture = TextureManager::Get("test");
  EXPECT_NE(nullptr, test_texture);
}
}  // namespace