#pragma once

#include <string>
#include <vector>

#include <SDL3_mixer/SDL_mixer.h>

namespace rse {
class Sound;  // forward declaration for Sound
class Music;  // forward declaration for Music

class SoundManager {
 public:
  static void Init();
  static void Add(Sound* sound);
  static void Add(Music* music);
  static Sound* GetSound(const std::string& name);
  static Music* GetMusic(const std::string& name);
  static void Clean();

  // SDL3_mixer has no global mixer: the engine owns one and shares it with
  // every Sound/Music instance.
  static MIX_Mixer* GetMixer() { return s_mixer; }

 private:
  SoundManager() {}
  static std::vector<Sound*> s_sounds;
  static std::vector<Music*> s_music;
  static MIX_Mixer* s_mixer;
};
}  // namespace rse
