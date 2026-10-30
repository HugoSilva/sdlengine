#pragma once

#include <algorithm>

#include "base_system.h"
#include <ecs/components/audio.h>
#include <ecs/ecs_manager.h>

// Applies each entity's playback settings to the sound it borrowed, and starts
// the ones that ask to autoplay. The settings live on the component so they
// survive a save and load; a Sound is shared, so the mixer track behind it is
// one track -- playing the same file from two entities at once shares it.
class AudioSystem : public BaseSystem {
 public:
  AudioSystem() : BaseSystem() {}

  void Update(const float alpha) {
    rse::ECSManager::GetView<AudioComponent>().each([](auto entity, AudioComponent& audio) {
      rse::Sound* sound = audio.m_sound.get();

      if (sound == nullptr) {
        return;
      }

      // A disabled entity or audio component is not in the scene, so a sound
      // it already started has to stop. Clearing the flag lets autoplay start
      // it again when it is re-enabled. Sound is shared by every entity that
      // names the same file, so this stops that one track.
      if (!rse::ECSManager::IsEntityEnabled(entity) || !rse::ECSManager::IsComponentEnabled<AudioComponent>(entity)) {
        if (audio.m_autoplay_started) {
          sound->Stop();
          audio.m_autoplay_started = false;
        }
        return;
      }

      sound->SetVolume(audio.mute ? 0 : std::clamp(audio.volume, 0, 128));

      if (audio.autoplay && !audio.m_autoplay_started) {
        audio.m_autoplay_started = true;

        if (audio.loop) {
          sound->Loop();
        } else {
          sound->Play();
        }
      } else if (!audio.autoplay) {
        // Arming it again from the inspector has to play it again.
        audio.m_autoplay_started = false;
      }
    });
  }
};
