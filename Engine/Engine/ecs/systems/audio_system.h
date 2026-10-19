#pragma once

#include "base_system.h"
#include <ecs/ecs_manager.h>

class AudioSystem : public BaseSystem {
 public:
  AudioSystem() : BaseSystem() {}

  void Update(const float alpha) {
    rse::ECSManager::GetView<TransformComponent, AudioComponent>().each(
        [&, this](auto entity, TransformComponent& transform, AudioComponent& audio) {
          // TODO Remove the logic from the old Audio object and do it here
          // transform, audio
        });
  }
};