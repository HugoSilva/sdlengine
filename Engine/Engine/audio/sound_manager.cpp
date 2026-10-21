#include "sound_manager.h"

#include <core.h>
#include <utility>

#include <SDL3/SDL.h>

#include "music.h"
#include "sound.h"
#include <core/asset_registry.h>
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

Sound* SoundManager::GetSound(const std::string& reference) {
  for (const std::unique_ptr<Sound>& sound : s_sounds) {
    // An id and a name are both references to the same sound: a scene that
    // stores the id and an inspector that shows the name have to land on it.
    if (sound->GetAssetId() == reference || sound->GetName() == reference) return sound.get();
  }
  return nullptr;
}

Music* SoundManager::GetMusic(const std::string& name) {
  for (const std::unique_ptr<Music>& music : s_music) {
    if (music->GetName() == name) return music.get();
  }
  return nullptr;
}

Sound* SoundManager::GetOrLoad(const std::string& reference) {
  // An empty reference is "no sound", not a sound called nothing.
  if (reference.empty()) {
    return nullptr;
  }

  if (Sound* existing = GetSound(reference)) {
    return existing;
  }

  std::string asset_id;
  std::string name = reference;
  std::string path;

  if (const AssetRegistry::Asset* asset = AssetRegistry::FindReference(reference)) {
    // A registered asset: the id is what the scene will save, the name is what
    // the inspector shows, and the registry's path is where the file is.
    asset_id = asset->id;
    name = asset->name;
    path = Core::ResolveProjectPath(asset->path);
  } else {
    // Not registered: the reference is a path, relative to the asset root,
    // exactly as it was before the registry existed.
    path = Core::ResolveResource(reference);
  }

  auto sound = std::make_unique<Sound>(asset_id, name, path);
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
