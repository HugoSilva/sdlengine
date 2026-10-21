#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include <SDL3_mixer/SDL_mixer.h>

namespace rse {
class Sound;  // forward declaration for Sound
class Music;  // forward declaration for Music

// Owns every sound and every piece of music in the project. Components borrow
// sounds: a scene stores the asset's id (or, for a file with no registry entry,
// its name), and the manager hands out the one Sound that reference resolved to,
// so two entities using one file share one decoded copy.
class SoundManager {
 public:
  static void Init();

  // Take ownership. Callers construct the object and hand it over; nothing
  // else may delete it afterwards.
  static void Add(std::unique_ptr<Sound> sound);
  static void Add(std::unique_ptr<Music> music);

  // The sound or piece of music registered under a name, or nullptr.
  static Sound* GetSound(const std::string& reference);
  static Music* GetMusic(const std::string& name);

  // The sound for `reference` -- an asset id, an asset name, or a bare path --
  // loading it the first time. A reference the asset registry knows resolves
  // through the registry, so the sound carries the asset id a scene saves. A
  // file that cannot be read still yields a Sound carrying the name, which
  // stays silent, so a scene referring to a missing file still opens.
  static Sound* GetOrLoad(const std::string& reference);

  // How many sounds the manager owns. For tests and diagnostics.
  static std::size_t Count();

  // Destroys everything it owns before tearing the mixer down. Safe to call
  // twice, and safe in a process that never called Init().
  static void Clean();

  // SDL3_mixer has no global mixer: the engine owns one and shares it with
  // every Sound/Music instance.
  static MIX_Mixer* GetMixer() { return s_mixer; }

 private:
  SoundManager() {}
  static std::vector<std::unique_ptr<Sound>> s_sounds;
  static std::vector<std::unique_ptr<Music>> s_music;
  static MIX_Mixer* s_mixer;
  static bool s_initialized;
};
}  // namespace rse
