#pragma once

#include <components/sprite.h>

namespace physics {
class Updatable {
 public:
  Updatable() {}

  virtual ~Updatable() {}

  virtual void Update() {}
};
}  // namespace physics