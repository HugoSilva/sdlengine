#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include <SDL3/SDL.h>
#include <gtest/gtest.h>

#include <audio/sound.h>
#include <audio/sound_manager.h>
#include <ecs/components/include_list.h>
#include <ecs/entity.h>
#include <ecs/scene.h>
#include <ecs/systems/audio_system.h>

/*
The other audio tests run without a mixer, which keeps them about identity and
lifetime. These ones need a mixer, because what is being tested is that one file
can play twice at once: a dummy audio device gives real tracks without needing a
sound card, and the tracks are created lazily, so the cost is two tracks for the
two copies a test asks for.

The WAV is written here rather than checked in: it is silence, and its only job
is to be a valid file the mixer can decode and play long enough to observe.
*/
namespace {

using namespace rse;

// Seconds of silence, written as a 44.1 kHz 16-bit mono PCM WAV.
std::filesystem::path WriteSilentWave(const std::filesystem::path& path, int milliseconds) {
  constexpr int kSampleRate = 44100;
  constexpr int kChannels = 1;
  constexpr int kBitsPerSample = 16;
  const int frames = kSampleRate * milliseconds / 1000;
  const int data_size = frames * kChannels * (kBitsPerSample / 8);

  std::ofstream file(path, std::ios::binary | std::ios::trunc);
  auto write_u32 = [&file](std::uint32_t value) { file.write(reinterpret_cast<const char*>(&value), sizeof(value)); };
  auto write_u16 = [&file](std::uint16_t value) { file.write(reinterpret_cast<const char*>(&value), sizeof(value)); };

  file.write("RIFF", 4);
  write_u32(static_cast<std::uint32_t>(36 + data_size));
  file.write("WAVE", 4);
  file.write("fmt ", 4);
  write_u32(16);
  write_u16(1);  // PCM
  write_u16(kChannels);
  write_u32(kSampleRate);
  write_u32(kSampleRate * kChannels * (kBitsPerSample / 8));
  write_u16(kChannels * (kBitsPerSample / 8));
  write_u16(kBitsPerSample);
  file.write("data", 4);
  write_u32(static_cast<std::uint32_t>(data_size));

  const std::vector<char> silence(static_cast<std::size_t>(data_size), 0);
  file.write(silence.data(), static_cast<std::streamsize>(silence.size()));

  return path;
}

class AudioVoiceTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // The dummy device has no hardware behind it, so the test runs on a build
    // machine and in CI. It has to be chosen before SDL starts audio.
    SDL_SetHint(SDL_HINT_AUDIO_DRIVER, "dummy");
    if (!SDL_Init(SDL_INIT_AUDIO)) {
      GTEST_SKIP() << "SDL audio is unavailable: " << SDL_GetError();
    }

    m_sdl_started = true;
    SoundManager::Init();
    if (SoundManager::GetMixer() == nullptr) {
      GTEST_SKIP() << "SDL_mixer could not create a mixer";
    }

    m_initialized = true;
    m_directory = std::filesystem::temp_directory_path() / "rse_audio_voice_test";
    std::filesystem::remove_all(m_directory);
    std::filesystem::create_directories(m_directory);
    m_file = WriteSilentWave(m_directory / "voice.wav", 2000);
  }

  void TearDown() override {
    // The sound owns mixer tracks; they have to go before the mixer does.
    m_sound.reset();

    if (m_initialized) SoundManager::Clean();
    if (m_sdl_started) SDL_QuitSubSystem(SDL_INIT_AUDIO);
    if (!m_directory.empty()) std::filesystem::remove_all(m_directory);
  }

  Sound& MakeSound() {
    m_sound = std::make_unique<Sound>("voice", m_file.string());
    return *m_sound;
  }

  std::unique_ptr<Sound> m_sound;
  std::filesystem::path m_directory;
  std::filesystem::path m_file;
  bool m_sdl_started{false};
  bool m_initialized{false};
};

TEST_F(AudioVoiceTest, TwoCopiesOfOneFileEachGetAVoice) {
  Sound& sound = MakeSound();

  const Sound::Voice first = sound.Play();
  const Sound::Voice second = sound.Play();

  ASSERT_TRUE(first.IsValid());
  ASSERT_TRUE(second.IsValid());
  EXPECT_NE(first.index, second.index);
  EXPECT_TRUE(sound.IsPlaying(first));
  EXPECT_TRUE(sound.IsPlaying(second));
  EXPECT_EQ(2, sound.PlayingVoiceCount());
}

TEST_F(AudioVoiceTest, StoppingOneCopyLeavesTheOtherPlaying) {
  Sound& sound = MakeSound();

  const Sound::Voice first = sound.Play();
  const Sound::Voice second = sound.Play();
  ASSERT_TRUE(first.IsValid());
  ASSERT_TRUE(second.IsValid());

  sound.Stop(first);

  EXPECT_FALSE(sound.IsPlaying(first));
  EXPECT_TRUE(sound.IsPlaying(second));
  EXPECT_EQ(1, sound.PlayingVoiceCount());
}

TEST_F(AudioVoiceTest, EachCopyKeepsItsOwnVolume) {
  Sound& sound = MakeSound();

  const Sound::Voice first = sound.Play();
  const Sound::Voice second = sound.Play();
  ASSERT_TRUE(first.IsValid());
  ASSERT_TRUE(second.IsValid());

  sound.SetVolume(first, 32);
  sound.SetVolume(second, 96);

  EXPECT_EQ(32, sound.GetVolume(first));
  EXPECT_EQ(96, sound.GetVolume(second));
  // The sound's default is what a new copy starts at; a scene's per-entity
  // levels do not rewrite it.
  EXPECT_EQ(128, sound.GetVolume());
}

TEST_F(AudioVoiceTest, TheNinthCopyTakesTheOldestVoice) {
  Sound& sound = MakeSound();
  std::vector<Sound::Voice> voices;

  for (int i = 0; i < Sound::kMaxVoices; ++i) {
    const Sound::Voice voice = sound.Play();
    ASSERT_TRUE(voice.IsValid());
    voices.push_back(voice);
  }
  ASSERT_EQ(Sound::kMaxVoices, sound.PlayingVoiceCount());

  const Sound::Voice stolen = sound.Play();
  ASSERT_TRUE(stolen.IsValid());

  // The oldest copy made room for the new one, and the handle it was given no
  // longer names the track, so its owner cannot change the new copy's volume.
  EXPECT_EQ(voices.front().index, stolen.index);
  EXPECT_NE(voices.front(), stolen);
  EXPECT_FALSE(sound.IsPlaying(voices.front()));
  EXPECT_EQ(-1, sound.GetVolume(voices.front()));
  EXPECT_EQ(Sound::kMaxVoices, sound.PlayingVoiceCount());
}

TEST_F(AudioVoiceTest, TwoEntitiesPlayingOneFileAreBothHeard) {
  Sound& sound = MakeSound();
  Scene scene;

  const uint32_t first = scene.CreateEntity().Handle();
  const uint32_t second = scene.CreateEntity().Handle();
  scene.Registry().assign<AudioComponent>(first, &sound);
  scene.Registry().assign<AudioComponent>(second, &sound);
  scene.Registry().get<AudioComponent>(first).autoplay = true;
  scene.Registry().get<AudioComponent>(second).autoplay = true;

  AudioSystem audio_system;
  audio_system.UpdateSimulation(scene, 1.0f / 60.0f);

  const AudioComponent& first_audio = scene.Registry().get<AudioComponent>(first);
  const AudioComponent& second_audio = scene.Registry().get<AudioComponent>(second);

  ASSERT_TRUE(first_audio.m_voice.IsValid());
  ASSERT_TRUE(second_audio.m_voice.IsValid());
  EXPECT_NE(first_audio.m_voice, second_audio.m_voice);
  EXPECT_TRUE(sound.IsPlaying(first_audio.m_voice));
  EXPECT_TRUE(sound.IsPlaying(second_audio.m_voice));
  EXPECT_EQ(2, sound.PlayingVoiceCount());
}

TEST_F(AudioVoiceTest, DisablingOneEntityLeavesTheOtherAudible) {
  Sound& sound = MakeSound();
  Scene scene;

  const uint32_t first = scene.CreateEntity().Handle();
  const uint32_t second = scene.CreateEntity().Handle();
  scene.Registry().assign<AudioComponent>(first, &sound);
  scene.Registry().assign<AudioComponent>(second, &sound);
  scene.Registry().get<AudioComponent>(first).autoplay = true;
  scene.Registry().get<AudioComponent>(second).autoplay = true;

  AudioSystem audio_system;
  audio_system.UpdateSimulation(scene, 1.0f / 60.0f);

  const Sound::Voice first_voice = scene.Registry().get<AudioComponent>(first).m_voice;
  const Sound::Voice second_voice = scene.Registry().get<AudioComponent>(second).m_voice;
  ASSERT_TRUE(first_voice.IsValid());
  ASSERT_TRUE(second_voice.IsValid());

  scene.SetEntityEnabled(first, false);
  audio_system.UpdateSimulation(scene, 1.0f / 60.0f);

  EXPECT_FALSE(sound.IsPlaying(first_voice));
  EXPECT_TRUE(sound.IsPlaying(second_voice));
  EXPECT_EQ(1, sound.PlayingVoiceCount());
}

TEST_F(AudioVoiceTest, TwoEntitiesKeepTheirOwnVolumes) {
  Sound& sound = MakeSound();
  Scene scene;

  const uint32_t first = scene.CreateEntity().Handle();
  const uint32_t second = scene.CreateEntity().Handle();
  scene.Registry().assign<AudioComponent>(first, &sound);
  scene.Registry().assign<AudioComponent>(second, &sound);
  scene.Registry().get<AudioComponent>(first).autoplay = true;
  scene.Registry().get<AudioComponent>(first).volume = 32;
  scene.Registry().get<AudioComponent>(second).autoplay = true;
  scene.Registry().get<AudioComponent>(second).volume = 96;

  AudioSystem audio_system;
  audio_system.UpdateSimulation(scene, 1.0f / 60.0f);

  const Sound::Voice first_voice = scene.Registry().get<AudioComponent>(first).m_voice;
  const Sound::Voice second_voice = scene.Registry().get<AudioComponent>(second).m_voice;
  ASSERT_TRUE(first_voice.IsValid());
  ASSERT_TRUE(second_voice.IsValid());

  EXPECT_EQ(32, sound.GetVolume(first_voice));
  EXPECT_EQ(96, sound.GetVolume(second_voice));

  // The project's mix changing must not rewrite either entity's level.
  SoundManager::SetVolumes(64, 128, 128);
  EXPECT_EQ(32, sound.GetVolume(first_voice));
  EXPECT_EQ(96, sound.GetVolume(second_voice));
}

}  // namespace
