#include "SoundManager.hpp"

#include <SDL3/SDL.h>

#include "../Utils/Logger.hpp"
#include "Music.hpp"
#include "Sound.hpp"

namespace rse {
std::vector<Sound*> SoundManager::s_sounds;
std::vector<Music*> SoundManager::s_music;
MIX_Mixer* SoundManager::s_mixer = nullptr;

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

  s_mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
  if (s_mixer == nullptr) {
    Logger::Error("SDL_mixer could not create a mixer, " + std::string(SDL_GetError()));
  }
}

void SoundManager::Add(Sound* sound) { s_sounds.push_back(sound); }

void SoundManager::Add(Music* music) { s_music.push_back(music); }

Sound* SoundManager::GetSound(const std::string& name) {
  for (Sound* sound : s_sounds) {
    if (sound->GetName() == name) return sound;
  }
  return nullptr;
}

Music* SoundManager::GetMusic(const std::string& name) {
  for (Music* music : s_music) {
    if (music->GetName() == name) return music;
  }
  return nullptr;
}

void SoundManager::Clean() {
  for (unsigned int i = 0; i < s_sounds.size(); i++) delete s_sounds[i];
  s_sounds.clear();

  for (unsigned int i = 0; i < s_music.size(); i++) delete s_music[i];
  s_music.clear();

  // MIX_Quit() tears down every mixer and MIX_Audio created by SDL3_mixer.
  s_mixer = nullptr;
  MIX_Quit();
}
}  // namespace rse
