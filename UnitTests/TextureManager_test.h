#pragma once

#include <gtest/gtest.h>
#include <Components/TextureManager.h>

/*
TODO I need to change the default behavior for constructing and calling Texture objects currently
it never fails but I should but a proper logic that would crash if something is missing, this is going
to be done after the new ECS comes into place
*/
namespace {

	TEST(TextureManager, AddTexture)
	{
		TextureManager::add(new Texture("test", "test.png"));
		Texture* testTexture = TextureManager::get("test");
		EXPECT_NE(nullptr, testTexture);
	}

	TEST(TextureManager, AddTexturePNGFormat)
	{
		TextureManager::add(new Texture("test", "test.png"));
		Texture* testTexture = TextureManager::get("test");
		EXPECT_NE(nullptr, testTexture);
	}

	TEST(TextureManager, AddTextureJPEGFormat)
	{
		TextureManager::add(new Texture("test", "test.jpeg"));
		Texture* testTexture = TextureManager::get("test");
		EXPECT_NE(nullptr, testTexture);
	}
}