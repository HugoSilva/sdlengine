#pragma once

#include <array>
#include <cstdint>
#include <string>

#include <SDL3_mixer/SDL_mixer.h>

namespace rse {
class Sound {
 public:
  // How many copies of one file may be in flight at once. Two entities that
  // play the same effect each need a track of their own; a ninth simultaneous
  // copy steals the oldest, because a mixer has a budget and a lost one-shot is
  // less wrong than a copy that never stops.
  static constexpr int kMaxVoices{8};

  // A handle to one Play() or Loop() call. It carries a generation so a handle
  // whose voice has since been reused -- the clip ended and another entity took
  // the track -- no longer names anything, and a stale volume update cannot
  // land on the new owner.
  struct Voice {
    int index{-1};
    std::uint32_t generation{0};

    bool IsValid() const { return index >= 0; }

    friend bool operator==(const Voice& a, const Voice& b) {
      return a.index == b.index && a.generation == b.generation;
    }
    friend bool operator!=(const Voice& a, const Voice& b) { return !(a == b); }
  };

  Sound(const std::string& name, const std::string& filename);
  Sound(const std::string& asset_id, const std::string& name, const std::string& filename);
  ~Sound();

  // Starts a copy on a free voice and returns a handle to it. Two entities that
  // play one file at the same time get two voices, so both are heard, and
  // stopping one leaves the other playing. The handle is invalid when the sound
  // has no mixer to play on: no mixer in the process, or a file that failed to
  // load.
  Voice Play();
  Voice Loop();

  // Pause, resume or stop the one copy behind a handle. A handle whose voice is
  // gone does nothing, so an entity that is disabled after its one-shot ended
  // does not silence whoever borrowed the track next.
  void Pause(Voice voice);
  void Resume(Voice voice);
  void Stop(Voice voice);

  // The level one copy plays at. Two entities on the same file keep their own.
  // Returns -1 for a handle whose voice no longer exists.
  void SetVolume(Voice voice, int volume);
  int GetVolume(Voice voice) const;

  // The sound as a whole, for callers that hold the Sound rather than one copy
  // of it: SoundManager, the editor, the tests.
  void PauseAll();
  void ResumeAll();
  void StopAll();
  void SetVolume(int volume);
  int GetVolume() const { return m_volume; }

  // Recomputes every voice's gain from its stored level, for when the project's
  // mix changes. The levels themselves are left alone.
  void RefreshGain();

  // True while any copy is playing. The voice overload is the one an entity
  // uses: its own copy ending is not the file ending.
  bool IsPlaying() const;
  bool IsPlaying(Voice voice) const;
  int PlayingVoiceCount() const;

  inline const std::string& GetAssetId() const { return m_asset_id; }
  inline const std::string& GetName() const { return m_name; }
  inline const std::string& GetFileName() const { return m_filename; }

 private:
  // One mixer track and the playback state that belongs to it. Tracks are
  // created on first use, so a sound that never overlaps costs one track and a
  // project with many one-shots does not pay for voices it never needs.
  struct VoiceSlot {
    MIX_Track* track = nullptr;
    int volume = 128;
    std::uint32_t generation{0};
    std::uint64_t started{0};
  };

  bool EnsureTrack(VoiceSlot& slot);
  int AcquireVoice();
  bool StartVoice(int index, bool loop);
  bool IsCurrent(Voice voice) const;

  // Empty when the sound came from a bare path rather than a registry entry.
  std::string m_asset_id;
  std::string m_name;
  std::string m_filename;
  // SDL3_mixer: preloaded audio, shared by every voice.
  MIX_Audio* m_sound = nullptr;
  std::array<VoiceSlot, kMaxVoices> m_voices{};
  std::uint64_t m_sequence{0};
  int m_next_voice{0};
  // The level a new voice starts at.
  int m_volume = 128;
};
}  // namespace rse
