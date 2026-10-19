#pragma once

#include <box2d/box2d.h>

#include "../Components/Sprite.hpp"
#include "Updatable.h"

class Collider : public physics::Updatable {
 public:
  Collider();
  void Update() override;

 private:
  b2Body* m_body;
};