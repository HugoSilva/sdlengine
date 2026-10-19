#pragma once

#include <gtest/gtest.h>

#include <audio/music.h>
#include <audio/sound.h>
#include <audio/sound_manager.h>

/*
TODO I need to change the default behavior for constructing and calling Sound/Music objects currently
it never fails but I should but a proper logic that would crash if something is missing, this is going
to be done after the new ECS comes into place
*/
namespace {
using namespace rse;

TEST(SoundManagerTest, AddSound) {
  SoundManager::Add(new Sound("test", "test.ogg"));
  Sound* test_sound = SoundManager::GetSound("test");
  EXPECT_NE(nullptr, test_sound);
}

TEST(SoundManagerTest, AddMusic) {
  SoundManager::Add(new Music("test", "test.ogg"));
  Music* test_music = SoundManager::GetMusic("test");
  EXPECT_NE(nullptr, test_music);
}

TEST(SoundManagerTest, AddSoundWaveFormat) {
  SoundManager::Add(new Music("test", "test.wav"));
  Sound* test_sound = SoundManager::GetSound("test");
  EXPECT_NE(nullptr, test_sound);
}

TEST(SoundManagerTest, AddSoundOggFormat) {
  SoundManager::Add(new Music("test", "test.ogg"));
  Sound* test_sound = SoundManager::GetSound("test");
  EXPECT_NE(nullptr, test_sound);
}
}  // namespace