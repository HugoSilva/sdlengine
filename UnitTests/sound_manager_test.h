#pragma once

#include <filesystem>
#include <memory>

#include <gtest/gtest.h>

#include <audio/music.h>
#include <audio/sound.h>
#include <audio/sound_manager.h>
#include <core/asset_registry.h>

/*
Sounds are loaded once per asset and owned by the manager; a component only
borrows one. No mixer is created here, so every Sound is valid but silent --
which is the same state a missing file leaves behind in an app that does have
a mixer.
*/
namespace {
using namespace rse;

class SoundManagerTest : public ::testing::Test {
 protected:
  void TearDown() override {
    SoundManager::Clean();
    AssetRegistry::Clean();
  }
};

TEST_F(SoundManagerTest, AddSound) {
  SoundManager::Add(std::make_unique<Sound>("test", "test.ogg"));

  EXPECT_NE(nullptr, SoundManager::GetSound("test"));
  EXPECT_EQ(1u, SoundManager::Count());
}

TEST_F(SoundManagerTest, AddSoundWaveFormat) {
  SoundManager::Add(std::make_unique<Sound>("wave", "test.wav"));
  SoundManager::Add(std::make_unique<Sound>("ogg", "test.ogg"));

  EXPECT_NE(nullptr, SoundManager::GetSound("wave"));
  EXPECT_NE(nullptr, SoundManager::GetSound("ogg"));
  EXPECT_EQ(2u, SoundManager::Count());
}

TEST_F(SoundManagerTest, AddMusic) {
  SoundManager::Add(std::make_unique<Music>("test", "test.ogg"));

  EXPECT_NE(nullptr, SoundManager::GetMusic("test"));
  // Music is not a sound: the component side only ever looks sounds up.
  EXPECT_EQ(nullptr, SoundManager::GetSound("test"));
}

TEST_F(SoundManagerTest, GetOfAnUnknownNameIsNull) {
  EXPECT_EQ(nullptr, SoundManager::GetSound("nothing_was_registered"));
  EXPECT_EQ(nullptr, SoundManager::GetMusic("nothing_was_registered"));
}

TEST_F(SoundManagerTest, GetOrLoadLoadsANameOnce) {
  Sound* first = SoundManager::GetOrLoad("shared.wav");
  Sound* second = SoundManager::GetOrLoad("shared.wav");

  ASSERT_NE(nullptr, first);
  EXPECT_EQ(first, second);
  EXPECT_EQ(first, SoundManager::GetSound("shared.wav"));
  EXPECT_EQ(1u, SoundManager::Count());
}

TEST_F(SoundManagerTest, GetOrLoadKeepsOneSoundPerName) {
  Sound* effect = SoundManager::GetOrLoad("shared.wav");
  Sound* music = SoundManager::GetOrLoad("other.wav");

  EXPECT_NE(effect, music);
  EXPECT_EQ(2u, SoundManager::Count());
}

TEST_F(SoundManagerTest, AMissingFileKeepsTheName) {
  Sound* sound = SoundManager::GetOrLoad("this_file_does_not_exist.wav");

  ASSERT_NE(nullptr, sound);
  EXPECT_EQ("this_file_does_not_exist.wav", sound->GetName());
  EXPECT_FALSE(sound->IsPlaying());
}

TEST_F(SoundManagerTest, AnEmptyNameMeansNoSound) { EXPECT_EQ(nullptr, SoundManager::GetOrLoad("")); }

TEST_F(SoundManagerTest, CleanTwiceIsSafe) {
  SoundManager::GetOrLoad("shared.wav");
  SoundManager::Add(std::make_unique<Music>("theme", "theme.ogg"));

  SoundManager::Clean();
  EXPECT_EQ(0u, SoundManager::Count());
  EXPECT_EQ(nullptr, SoundManager::GetSound("shared.wav"));

  SoundManager::Clean();
  EXPECT_EQ(0u, SoundManager::Count());
}

// With a registry open, a reference resolves to an asset: the sound carries the
// id a scene saves and the name the inspector shows, and the id and the name
// are the same sound.
TEST_F(SoundManagerTest, ARegisteredReferenceLoadsByIdAndKeepsTheName) {
  const std::filesystem::path directory = std::filesystem::temp_directory_path() / "rse_sound_registry_test";
  std::filesystem::remove_all(directory);
  std::filesystem::create_directories(directory / "assets");

  AssetRegistry::Init(directory);
  const std::string id = AssetRegistry::Add("effect.wav", "sound", "assets/effect.wav");

  Sound* by_id = SoundManager::GetOrLoad(id);
  ASSERT_NE(nullptr, by_id);
  EXPECT_EQ(by_id->GetAssetId(), id);
  EXPECT_EQ(by_id->GetName(), "effect.wav");
  EXPECT_EQ(1u, SoundManager::Count());

  EXPECT_EQ(by_id, SoundManager::GetSound("effect.wav"));
  EXPECT_EQ(by_id, SoundManager::GetOrLoad("effect.wav"));
  EXPECT_EQ(1u, SoundManager::Count());

  std::filesystem::remove_all(directory);
}
}  // namespace
