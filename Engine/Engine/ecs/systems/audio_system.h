#pragma once

#include <algorithm>

#include "base_system.h"
#include <ecs/components/audio.h>
#include <ecs/scene.h>

// Applies each entity's playback settings to the sound it borrowed, and starts
// the ones that ask to autoplay. The settings live on the component so they
// survive a save and load; the Sound is shared, but every entity that plays it
// gets a voice of its own, so two entities on one file are both heard.
class AudioSystem : public BaseSystem {
 public:
  AudioSystem() : BaseSystem() {}

  // Playback settings are applied in the simulation phase, so a stopped scene
  // stays silent. The work is idempotent, which is what makes running it in
  // fixed steps -- several times a frame, or not at all on a fast display --
  // harmless.
  void UpdateSimulation(rse::Scene& scene, const float step) override {
    (void)step;
    scene.Registry().view<AudioComponent>().each([&scene](auto entity, AudioComponent& audio) {
      rse::Sound* sound = audio.m_sound.get();

      if (sound == nullptr) {
        return;
      }

      // A disabled entity or audio component is not in the scene, so the copy
      // it started has to stop. Only that copy: another entity playing the same
      // file has its own voice and keeps playing.
      if (!scene.IsEntityEnabled(entity) || !scene.IsComponentEnabled<AudioComponent>(entity)) {
        sound->Stop(audio.m_voice);
        audio.m_voice = {};
        audio.m_autoplay_started = false;
        return;
      }

      const int volume = audio.mute ? 0 : std::clamp(audio.volume, 0, rse::SoundManager::kMaxVolume);

      if (audio.autoplay && !audio.m_autoplay_started) {
        audio.m_autoplay_started = true;
        audio.m_voice = audio.loop ? sound->Loop() : sound->Play();
      } else if (!audio.autoplay) {
        // Arming it again from the inspector has to play it again, and a loop
        // that is still running has to be reachable: turning autoplay off
        // stops the copy this component started.
        if (audio.m_autoplay_started) sound->Stop(audio.m_voice);
        audio.m_voice = {};
        audio.m_autoplay_started = false;
      }

      // A voice keeps the level its entity asked for. If the copy has ended and
      // somebody else took the track, the handle no longer matches and this is
      // a no-op, so the new owner's volume is safe.
      if (audio.m_autoplay_started) {
        sound->SetVolume(audio.m_voice, volume);
      }
    });
  }
};
