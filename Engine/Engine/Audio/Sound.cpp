#include "Sound.hpp"

#include "../Utils/Logger.hpp"
#include "SoundManager.hpp"

namespace rse {
Sound::Sound(const std::string& name, const std::string& filename) : m_Name(name), m_Filename(filename) {
  MIX_Mixer* mixer = SoundManager::GetMixer();
  if (mixer == nullptr) {
    // No mixer yet (e.g. headless unit tests): keep the object valid but silent.
    return;
  }

  // predecode = true: sound effects are short and played often.
  m_Sound = MIX_LoadAudio(mixer, filename.c_str(), true);
  if (m_Sound == nullptr) {
    Logger::Error("SDL_mixer could not load filename " + m_Filename);
    return;
  }

  m_Track = MIX_CreateTrack(mixer);
  if (m_Track != nullptr) {
    MIX_SetTrackAudio(m_Track, m_Sound);
    MIX_SetTrackGain(m_Track, static_cast<float>(m_Volume) / 128.0f);
  }
}

Sound::~Sound() {
  if (m_Track != nullptr) MIX_DestroyTrack(m_Track);
  if (m_Sound != nullptr) MIX_DestroyAudio(m_Sound);
}

void Sound::Play() {
  if (m_Track == nullptr) return;

  MIX_StopTrack(m_Track, 0);
  if (!MIX_PlayTrack(m_Track, 0)) {
    Logger::Error("SDL_mixer could not play filename " + m_Filename);
  }
}

void Sound::Loop() {
  if (m_Track == nullptr) return;

  SDL_PropertiesID options = SDL_CreateProperties();
  SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOPS_NUMBER, -1);

  MIX_StopTrack(m_Track, 0);
  if (!MIX_PlayTrack(m_Track, options)) {
    Logger::Error("SDL_mixer could not loop filename " + m_Filename);
  }

  SDL_DestroyProperties(options);
}

void Sound::Resume() {
  if (m_Track != nullptr) MIX_ResumeTrack(m_Track);
}

void Sound::Pause() {
  if (m_Track != nullptr) MIX_PauseTrack(m_Track);
}

void Sound::Stop() {
  if (m_Track != nullptr) MIX_StopTrack(m_Track, 0);
}

void Sound::SetVolume(int volume) {
  m_Volume = volume;
  if (m_Track != nullptr) MIX_SetTrackGain(m_Track, static_cast<float>(m_Volume) / 128.0f);
}
}  // namespace rse
