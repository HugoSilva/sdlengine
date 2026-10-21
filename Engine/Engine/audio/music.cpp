#include "music.h"

#include <algorithm>

#include "sound_manager.h"
#include <utils/logger.h>

namespace rse {
Music::Music(const std::string& name, const std::string& filename) : m_name(name), m_filename(filename) {
  MIX_Mixer* mixer = SoundManager::GetMixer();
  if (mixer == nullptr) {
    // No mixer yet (e.g. headless unit tests): keep the object valid but silent.
    return;
  }

  // predecode = false: stream long tracks instead of loading them fully.
  m_music = MIX_LoadAudio(mixer, filename.c_str(), false);
  if (m_music == nullptr) {
    Logger::Error("SDL_mixer could not load filename " + m_filename);
    return;
  }

  m_track = MIX_CreateTrack(mixer);
  if (m_track != nullptr) {
    MIX_SetTrackAudio(m_track, m_music);
    MIX_SetTrackGain(m_track, SoundManager::MusicGain(m_volume));
  }
}

Music::~Music() {
  if (m_track != nullptr) MIX_DestroyTrack(m_track);
  if (m_music != nullptr) MIX_DestroyAudio(m_music);
}

void Music::Play() {
  if (m_track == nullptr) return;

  // The project's mix may have changed since this track was created.
  MIX_SetTrackGain(m_track, SoundManager::MusicGain(m_volume));

  MIX_StopTrack(m_track, 0);
  if (!MIX_PlayTrack(m_track, 0)) {
    Logger::Error("SDL_mixer could not play filename " + m_filename);
  }
}

void Music::Loop() {
  if (m_track == nullptr) return;

  // The project's mix may have changed since this track was created.
  MIX_SetTrackGain(m_track, SoundManager::MusicGain(m_volume));

  SDL_PropertiesID options = SDL_CreateProperties();
  SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOPS_NUMBER, -1);

  MIX_StopTrack(m_track, 0);
  if (!MIX_PlayTrack(m_track, options)) {
    Logger::Error("SDL_mixer could not loop filename " + m_filename);
  }

  SDL_DestroyProperties(options);
}

void Music::Resume() {
  if (m_track != nullptr) MIX_ResumeTrack(m_track);
}

void Music::Pause() {
  if (m_track != nullptr) MIX_PauseTrack(m_track);
}

void Music::Stop() {
  if (m_track != nullptr) MIX_StopTrack(m_track, 0);
}

void Music::SetVolume(int volume) {
  m_volume = std::clamp(volume, 0, SoundManager::kMaxVolume);
  if (m_track != nullptr) MIX_SetTrackGain(m_track, SoundManager::MusicGain(m_volume));
}
}  // namespace rse
