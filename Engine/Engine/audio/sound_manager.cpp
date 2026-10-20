#include "sound_manager.h"

#include <core.h>
#include <utility>

#include <SDL3/SDL.h>

#include "music.h"
#include "sound.h"
#include <utils/logger.h>

namespace rse {
std::vector<std::unique_ptr<Sound>> SoundManager::s_sounds;
std::vector<std::unique_ptr<Music>> SoundManager::s_music;
MIX_Mixer* SoundManager::s_mixer = nullptr;
bool SoundManager::s_initialized = false;

void SoundManager::Init() {
  if (s_initialized) {
    return;
  }

  // SDL3_mixer: init the library, then create a mixer on the default device.
  if (!MIX_Init()) {
    Logger::Error("SDL_mixer could not initialize, " + std::string(SDL_GetError()));
    return;
  }

  s_initialized = true;

  SDL_AudioSpec spec{};
  spec.format = SDL_AUDIO_F32;
  spec.channels = 2;
  spec.freq = 44100;

  s_mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
  if (s_mixer == nullptr) {
    Logger::Error("SDL_mixer could not create a mixer, " + std::string(SDL_GetError()));
  }
}

void SoundManager::Add(std::unique_ptr<Sound> sound) { s_sounds.push_back(std::move(sound)); }

void SoundManager::Add(std::unique_ptr<Music> music) { s_music.push_back(std::move(music)); }

Sound* SoundManager::GetSound(const std::string& name) {
  for (const std::unique_ptr<Sound>& sound : s_sounds) {
    if (sound->GetName() == name) return sound.get();
  }
  return nullptr;
}

Music* SoundManager::GetMusic(const std::string& name) {
  for (const std::unique_ptr<Music>& music : s_music) {
    if (music->GetName() == name) return music.get();
  }
  return nullptr;
}

Sound* SoundManager::GetOrLoad(const std::string& name) {
  // An empty name is "no sound", not a sound called nothing.
  if (name.empty()) {
    return nullptr;
  }

  if (Sound* existing = GetSound(name)) {
    return existing;
  }

  auto sound = std::make_unique<Sound>(name, Core::ResolveResource(name));
  Sound* created = sound.get();
  s_sounds.push_back(std::move(sound));
  return created;
}

std::size_t SoundManager::Count() { return s_sounds.size(); }

void SoundManager::Clean() {
  // The objects first: their destructors still talk to the mixer.
  s_sounds.clear();
  s_music.clear();

  // MIX_Quit() matches MIX_Init(), so it is only called if this process
  // initialised the library -- the unit tests never do, and must not tear down
  // a mixer they never created.
  if (s_initialized) {
    MIX_Quit();
    s_initialized = false;
  }

  s_mixer = nullptr;
}
}  // namespace rse
