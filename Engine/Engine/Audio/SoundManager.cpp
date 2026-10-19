#include "SoundManager.hpp"

#include <SDL3/SDL.h>

#include "../Utils/Logger.hpp"
#include "Music.hpp"
#include "Sound.hpp"

namespace rse {
std::vector<Sound*> SoundManager::s_Sounds;
std::vector<Music*> SoundManager::s_Music;
MIX_Mixer* SoundManager::s_Mixer = nullptr;

void SoundManager::init() {
  // SDL3_mixer: init the library, then create a mixer on the default device.
  if (!MIX_Init()) {
    Logger::error("SDL_mixer could not initialize, " + std::string(SDL_GetError()));
    return;
  }

  SDL_AudioSpec spec{};
  spec.format = SDL_AUDIO_F32;
  spec.channels = 2;
  spec.freq = 44100;

  s_Mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
  if (s_Mixer == nullptr) {
    Logger::error("SDL_mixer could not create a mixer, " + std::string(SDL_GetError()));
  }
}

void SoundManager::add(Sound* sound) { s_Sounds.push_back(sound); }

void SoundManager::add(Music* music) { s_Music.push_back(music); }

Sound* SoundManager::getSound(const std::string& name) {
  for (Sound* sound : s_Sounds) {
    if (sound->getName() == name) return sound;
  }
  return nullptr;
}

Music* SoundManager::getMusic(const std::string& name) {
  for (Music* music : s_Music) {
    if (music->getName() == name) return music;
  }
  return nullptr;
}

void SoundManager::clean() {
  for (unsigned int i = 0; i < s_Sounds.size(); i++) delete s_Sounds[i];
  s_Sounds.clear();

  for (unsigned int i = 0; i < s_Music.size(); i++) delete s_Music[i];
  s_Music.clear();

  // MIX_Quit() tears down every mixer and MIX_Audio created by SDL3_mixer.
  s_Mixer = nullptr;
  MIX_Quit();
}
}  // namespace rse
