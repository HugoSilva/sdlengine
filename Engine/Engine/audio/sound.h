#pragma once

#include <string>

#include <SDL3_mixer/SDL_mixer.h>

namespace rse {
class Sound {
 public:
  Sound(const std::string& name, const std::string& filename);
  ~Sound();

  void Play();
  void Loop();
  void Pause();
  void Resume();
  void Stop();

  void SetVolume(int volume);

  inline const bool IsPlaying() const { return m_track != nullptr && MIX_TrackPlaying(m_track); }
  inline const int GetVolume() const { return m_volume; }
  inline const std::string& GetName() const { return m_name; }
  inline const std::string& GetFileName() const { return m_filename; }

  inline bool* GetMute() { return &m_mute; }
  inline bool* GetPlayTrigger() { return &m_play_trigger; }
  inline bool* GetLoop() { return &m_loop; }

 private:
  std::string m_name;
  std::string m_filename;
  // SDL3_mixer: preloaded audio + the track used to play it.
  MIX_Audio* m_sound = nullptr;
  MIX_Track* m_track = nullptr;
  int m_volume = 128;
  bool m_mute{false};
  bool m_play_trigger{true};
  bool m_loop{false};
};
}  // namespace rse
