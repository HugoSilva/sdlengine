#pragma once

#include <memory>
#include <string>

#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>

#include <audio/sound.h>
#include <audio/sound_manager.h>

// Sounds are owned by SoundManager; a component only borrows one. The
// shared_ptr keeps the existing null checks, but its deleter does nothing --
// deleting a sound the manager owns would be a double free the moment two
// entities use the same file.
inline std::shared_ptr<rse::Sound> BorrowSound(rse::Sound* sound) {
  return std::shared_ptr<rse::Sound>(sound, [](rse::Sound*) {});
}

// The playback settings belong to the entity, not to the shared Sound: two
// entities that use effect.wav can have different volumes, and one can loop
// while the other plays once. The sound itself is one asset, resolved by name
// through SoundManager.
struct AudioComponent {
  AudioComponent() {}

  AudioComponent(rse::Sound* sound) : m_sound(BorrowSound(sound)) {}

  std::shared_ptr<rse::Sound> m_sound{nullptr};

  int volume{128};  // 0..128, the range Sound::SetVolume takes
  bool mute{false};
  bool loop{false};
  bool autoplay{false};

  // Set by AudioSystem once an autoplay sound has been started, so a one-shot
  // effect is not restarted on every frame. Cleared while autoplay is off, so
  // ticking it again plays again. Runtime state, never saved.
  bool m_autoplay_started{false};

  template <typename Archive>
  void serialize(Archive& archive) {
    // Saving writes the sound's name; loading asks the manager for it. Same
    // split as a sprite's texture, and for the same reason: cereal writes a
    // pointer record for a shared_ptr and a string for a string.
    if constexpr (Archive::is_saving::value) {
      const std::string name = m_sound == nullptr ? "" : m_sound->GetName();
      archive(cereal::make_nvp("Sound", name), cereal::make_nvp("Volume", volume), cereal::make_nvp("Mute", mute),
              cereal::make_nvp("Loop", loop), cereal::make_nvp("Autoplay", autoplay));
    } else {
      std::string name;
      archive(cereal::make_nvp("Sound", name), cereal::make_nvp("Volume", volume), cereal::make_nvp("Mute", mute),
              cereal::make_nvp("Loop", loop), cereal::make_nvp("Autoplay", autoplay));
      m_sound = name.empty() ? nullptr : BorrowSound(rse::SoundManager::GetOrLoad(name));
    }
  }
};
