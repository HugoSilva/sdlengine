#pragma once

#include "clock.h"

namespace rse::platform {
class SdlClock : public Clock {
 public:
  std::uint64_t NowMilliseconds() const override;
};
}  // namespace rse::platform
