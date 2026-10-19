#include "SoundManager.hpp"

#include <SDL3/SDL.h>

#include "../Utils/Logger.hpp"
#include "Music.hpp"
#include "Sound.hpp"

namespace rse {
std::vector<Sound*> SoundManager::s_Sounds;
std::vector<Music*> SoundManager::s_Music;
MIX_Mixer* SoundManager::s_Mixer = nullptr;

void SoundManager::Init() {
  // SDL3_mixer: init the library, then create a mixer on the default device.
  if (!MIX_Init()) {
    Logger::Error("SDL_mixer could not initialize, " + std::string(SDL_GetError()));
    return;
  }

  SDL_AudioSpec spec{};
  spec.format = SDL_AUDIO_F32;
  spec.channels = 2;
  spec.freq = 44100;

  s_Mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
  if (s_Mixer == nullptr) {
    Logger::Error("SDL_mixer could not create a mixer, " + std::string(SDL_GetError()));
  }
}

void SoundManager::Add(Sound* sound) { s_Sounds.push_back(sound); }

void SoundManager::Add(Music* music) { s_Music.push_back(music); }

Sound* SoundManager::GetSound(const std::string& name) {
  for (Sound* sound : s_Sounds) {
    if (sound->GetName() == name) return sound;
  }
  return nullptr;
}

Music* SoundManager::GetMusic(const std::string& name) {
  for (Music* music : s_Music) {
    if (music->GetName() == name) return music;
  }
  return nullptr;
}

void SoundManager::Clean() {
  for (unsigned int i = 0; i < s_Sounds.size(); i++) delete s_Sounds[i];
  s_Sounds.clear();

  for (unsigned int i = 0; i < s_Music.size(); i++) delete s_Music[i];
  s_Music.clear();

  // MIX_Quit() tears down every mixer and MIX_Audio created by SDL3_mixer.
  s_Mixer = nullptr;
  MIX_Quit();
}
}  // namespace rse
