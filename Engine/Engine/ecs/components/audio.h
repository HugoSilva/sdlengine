#pragma once

#include <cereal/cereal.hpp>

#include <audio/sound.h>

struct AudioComponent {
  AudioComponent() {};

  AudioComponent(rse::Sound* sound) : m_sound(sound) {};

  std::shared_ptr<rse::Sound> m_sound;

  template <typename Archive>
  void serialize(Archive& archive) {
    archive(cereal::make_nvp("Sound", m_sound));
  }
};
