#pragma once

#include <cstdint>

namespace rse::platform {
// The engine's time source. Keeping this behind an interface means a console
// backend can provide its own tick counter without pulling SDL into Core's
// header or into gameplay code.
class Clock {
 public:
  virtual ~Clock() = default;
  virtual std::uint64_t NowMilliseconds() const = 0;
};
}  // namespace rse::platform
