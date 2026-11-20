#include "sound.h"

#include <algorithm>
#include <utility>

#include "sound_manager.h"
#include <utils/logger.h>

namespace rse {
Sound::Sound(const std::string& name, const std::string& filename) : Sound("", name, filename) {}

Sound::Sound(const std::string& asset_id, const std::string& name, const std::string& filename)
    : m_asset_id(asset_id), m_name(name), m_filename(filename) {
  MIX_Mixer* mixer = SoundManager::GetMixer();
  if (mixer == nullptr) {
    // No mixer yet (e.g. headless unit tests): keep the object valid but silent.
    return;
  }

  // predecode = true: sound effects are short and played often. The decoded
  // audio is what every voice of this sound shares.
  m_sound = MIX_LoadAudio(mixer, filename.c_str(), true);
  if (m_sound == nullptr) {
    Logger::Error("SDL_mixer could not load filename " + m_filename);
  }
}

Sound::~Sound() {
  for (VoiceSlot& slot : m_voices) {
    if (slot.track != nullptr) MIX_DestroyTrack(slot.track);
  }

  if (m_sound != nullptr) MIX_DestroyAudio(m_sound);
}

bool Sound::EnsureTrack(VoiceSlot& slot) {
  if (slot.track != nullptr) return true;

  MIX_Mixer* mixer = SoundManager::GetMixer();
  if (mixer == nullptr || m_sound == nullptr) return false;

  slot.track = MIX_CreateTrack(mixer);
  if (slot.track == nullptr) return false;

  if (!MIX_SetTrackAudio(slot.track, m_sound)) {
    MIX_DestroyTrack(slot.track);
    slot.track = nullptr;
    return false;
  }

  return true;
}

int Sound::AcquireVoice() {
  if (m_sound == nullptr) return -1;

  // A voice that is not playing never cuts another copy off, so one-shots
  // overlap instead of taking turns.
  for (int i = 0; i < kMaxVoices; ++i) {
    const int index = (m_next_voice + i) % kMaxVoices;
    MIX_Track* track = m_voices[index].track;
    if (track == nullptr || !MIX_TrackPlaying(track)) {
      m_next_voice = (index + 1) % kMaxVoices;
      return index;
    }
  }

  // Every voice is busy: steal the one that started first. The handle its
  // owner holds stops matching, so its volume updates do not follow the track.
  int oldest = 0;
  for (int i = 1; i < kMaxVoices; ++i) {
    if (m_voices[i].started < m_voices[oldest].started) oldest = i;
  }

  m_next_voice = (oldest + 1) % kMaxVoices;
  MIX_StopTrack(m_voices[oldest].track, 0);
  return oldest;
}

bool Sound::StartVoice(int index, bool loop) {
  VoiceSlot& slot = m_voices[index];
  if (!EnsureTrack(slot)) return false;

  // A new copy is a new handle, so whoever held the previous one cannot move
  // this voice's volume by accident.
  slot.generation += 1;
  slot.volume = m_volume;
  slot.started = ++m_sequence;

  // The project's mix may have changed since this sound was created.
  MIX_SetTrackGain(slot.track, SoundManager::SfxGain(slot.volume));
  MIX_StopTrack(slot.track, 0);

  bool played = false;

  if (loop) {
    SDL_PropertiesID options = SDL_CreateProperties();
    SDL_SetNumberProperty(options, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
    played = MIX_PlayTrack(slot.track, options);
    SDL_DestroyProperties(options);
  } else {
    played = MIX_PlayTrack(slot.track, 0);
  }

  if (!played) {
    Logger::Error("SDL_mixer could not " + std::string(loop ? "loop" : "play") + " filename " + m_filename);
  }

  return played;
}

bool Sound::IsCurrent(Voice voice) const {
  return voice.index >= 0 && voice.index < kMaxVoices && m_voices[voice.index].generation == voice.generation;
}

Sound::Voice Sound::Play() {
  const int index = AcquireVoice();
  if (index < 0) return {};

  if (!StartVoice(index, false)) return {};
  return Voice{index, m_voices[index].generation};
}

Sound::Voice Sound::Loop() {
  const int index = AcquireVoice();
  if (index < 0) return {};

  if (!StartVoice(index, true)) return {};
  return Voice{index, m_voices[index].generation};
}

void Sound::Pause(Voice voice) {
  if (!IsCurrent(voice)) return;
  MIX_Track* track = m_voices[voice.index].track;
  if (track != nullptr) MIX_PauseTrack(track);
}

void Sound::Resume(Voice voice) {
  if (!IsCurrent(voice)) return;
  MIX_Track* track = m_voices[voice.index].track;
  if (track != nullptr) MIX_ResumeTrack(track);
}

void Sound::Stop(Voice voice) {
  if (!IsCurrent(voice)) return;
  MIX_Track* track = m_voices[voice.index].track;
  if (track != nullptr) MIX_StopTrack(track, 0);
}

void Sound::PauseAll() {
  for (VoiceSlot& slot : m_voices) {
    if (slot.track != nullptr) MIX_PauseTrack(slot.track);
  }
}

void Sound::ResumeAll() {
  for (VoiceSlot& slot : m_voices) {
    if (slot.track != nullptr) MIX_ResumeTrack(slot.track);
  }
}

void Sound::StopAll() {
  for (VoiceSlot& slot : m_voices) {
    if (slot.track != nullptr) MIX_StopTrack(slot.track, 0);
  }
}

void Sound::SetVolume(Voice voice, int volume) {
  if (!IsCurrent(voice)) return;

  VoiceSlot& slot = m_voices[voice.index];
  slot.volume = std::clamp(volume, 0, SoundManager::kMaxVolume);
  if (slot.track != nullptr) MIX_SetTrackGain(slot.track, SoundManager::SfxGain(slot.volume));
}

int Sound::GetVolume(Voice voice) const {
  if (!IsCurrent(voice)) return -1;
  return m_voices[voice.index].volume;
}

void Sound::SetVolume(int volume) {
  m_volume = std::clamp(volume, 0, SoundManager::kMaxVolume);

  // Every copy of the sound moves with it, which is what "set this sound's
  // volume" means to a caller that does not hold a voice. A scene sets levels
  // per entity through the voice overload instead.
  for (VoiceSlot& slot : m_voices) {
    if (slot.track == nullptr) continue;
    slot.volume = m_volume;
    MIX_SetTrackGain(slot.track, SoundManager::SfxGain(slot.volume));
  }
}

void Sound::RefreshGain() {
  for (VoiceSlot& slot : m_voices) {
    if (slot.track != nullptr) MIX_SetTrackGain(slot.track, SoundManager::SfxGain(slot.volume));
  }
}

bool Sound::IsPlaying() const { return PlayingVoiceCount() > 0; }

bool Sound::IsPlaying(Voice voice) const {
  if (!IsCurrent(voice)) return false;
  MIX_Track* track = m_voices[voice.index].track;
  return track != nullptr && MIX_TrackPlaying(track);
}

int Sound::PlayingVoiceCount() const {
  int playing = 0;

  for (const VoiceSlot& slot : m_voices) {
    if (slot.track != nullptr && MIX_TrackPlaying(slot.track)) ++playing;
  }

  return playing;
}
}  // namespace rse
