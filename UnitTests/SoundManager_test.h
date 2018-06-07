#pragma once

#include <gtest/gtest.h>
#include <Audio/SoundManager.h>

namespace {
	using namespace audio;

	TEST(SoundManager, AddSound)
	{
		SoundManager::add(new Sound("test", "test.ogg"));
		Sound* testSound = SoundManager::getSound("test");
		EXPECT_NE(nullptr, testSound);
	}

	TEST(SoundManager, AddMusic)
	{
		SoundManager::add(new Music("test", "test.ogg"));
		Music* testMusic = SoundManager::getMusic("test");
		EXPECT_NE(nullptr, testMusic);
	}

	TEST(SoundManager, AddSoundWaveFormat)
	{
		SoundManager::add(new Music("test", "test.wav"));
		Sound* testSound = SoundManager::getSound("test");
		EXPECT_NE(nullptr, testSound);
	}

	TEST(SoundManager, AddSoundOggFormat)
	{
		SoundManager::add(new Music("test", "test.ogg"));
		Sound* testSound = SoundManager::getSound("test");
		EXPECT_NE(nullptr, testSound);
	}
}