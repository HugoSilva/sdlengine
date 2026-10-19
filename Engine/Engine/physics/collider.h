#pragma once

#include <box2d/box2d.h>

#include "updatable.h"
#include <components/sprite.h>

class Collider : public physics::Updatable {
 public:
  Collider();
  void Update() override;

 private:
  b2Body* m_body;
};