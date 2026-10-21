#pragma once

#include <string>

#include <SDL3_mixer/SDL_mixer.h>

namespace rse {
class Sound {
 public:
  Sound(const std::string& name, const std::string& filename);

  // A sound that knows its asset id: the id a scene stores, as opposed to the
  // name the inspector shows. The two-argument constructor is the same thing
  // with no id, which is what a sound loaded from a bare path has.
  Sound(const std::string& asset_id, const std::string& name, const std::string& filename);
  ~Sound();

  void Play();
  void Loop();
  void Pause();
  void Resume();
  void Stop();

  void SetVolume(int volume);

  inline const bool IsPlaying() const { return m_track != nullptr && MIX_TrackPlaying(m_track); }
  inline const int GetVolume() const { return m_volume; }
  inline const std::string& GetAssetId() const { return m_asset_id; }
  inline const std::string& GetName() const { return m_name; }
  inline const std::string& GetFileName() const { return m_filename; }

 private:
  // Empty when the sound came from a bare path rather than a registry entry.
  std::string m_asset_id;
  std::string m_name;
  std::string m_filename;
  // SDL3_mixer: preloaded audio + the track used to play it.
  MIX_Audio* m_sound = nullptr;
  MIX_Track* m_track = nullptr;
  int m_volume = 128;
};
}  // namespace rse
