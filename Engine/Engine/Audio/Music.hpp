#pragma once

#include <string>

#include <SDL3_mixer/SDL_mixer.h>

namespace rse {
class Music {
 public:
  Music(const std::string& name, const std::string& filename);
  ~Music();

  void play();
  void loop();
  void pause();
  void resume();
  void stop();

  void setVolume(int volume);

  inline const bool isPlaying() const { return m_Track != nullptr && MIX_TrackPlaying(m_Track); }
  inline const int getVolume() const { return m_Volume; }
  inline const std::string& getName() const { return m_Name; }
  inline const std::string& getFileName() const { return m_Filename; }

 private:
  std::string m_Name;
  std::string m_Filename;
  // SDL3_mixer: streaming audio + the track used to play it.
  MIX_Audio* m_Music = nullptr;
  MIX_Track* m_Track = nullptr;
  int m_Volume = 128;
};
}  // namespace rse
