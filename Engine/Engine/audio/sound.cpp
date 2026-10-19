#include "sound.h"

#include "sound_manager.h"
#include <utils/logger.h>

namespace rse {
Sound::Sound(const std::string& name, const std::string& filename) : m_name(name), m_filename(filename) {
  MIX_Mixer* mixer = SoundManager::GetMixer();
  if (mixer == nullptr) {
    // No mixer yet (e.g. headless unit tests): keep the object valid but silent.
    return;
  }

  // predecode = true: sound effects are short and played often.
  m_sound = MIX_LoadAudio(mixer, filename.c_str(), true);
  if (m_sound == nullptr) {
    Logger::Error("SDL_mixer could not load filename " + m_filename);
    return;
  }

  m_track = MIX_CreateTrack(mixer);
  if (m_track != nullptr) {
    MIX_SetTrackAudio(m_track, m_sound);
    MIX_SetTrackGain(m_track, static_cast<float>(m_volume) / 128.0f);
  }
}

Sound::~Sound() {
  if (m_track != nullptr) MIX_DestroyTrack(m_track);
  if (m_sound != nullptr) MIX_DestroyAudio(m_sound);
}

void Sound::Play() {
  if (m_track == nullptr) return;

  MIX_StopTrack(m_track, 0);
  if (!MIX_PlayTrack(m_track, 0)) {
    Logger::Error("SDL_mixer could not play filename " + m_filename);
  }
}

void Sound::Loop() {
  if (m_track == nullptr) return;

  SDL_PropertiesID options = SDL_CreateProperties();
  SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOPS_NUMBER, -1);

  MIX_StopTrack(m_track, 0);
  if (!MIX_PlayTrack(m_track, options)) {
    Logger::Error("SDL_mixer could not loop filename " + m_filename);
  }

  SDL_DestroyProperties(options);
}

void Sound::Resume() {
  if (m_track != nullptr) MIX_ResumeTrack(m_track);
}

void Sound::Pause() {
  if (m_track != nullptr) MIX_PauseTrack(m_track);
}

void Sound::Stop() {
  if (m_track != nullptr) MIX_StopTrack(m_track, 0);
}

void Sound::SetVolume(int volume) {
  m_volume = volume;
  if (m_track != nullptr) MIX_SetTrackGain(m_track, static_cast<float>(m_volume) / 128.0f);
}
}  // namespace rse
