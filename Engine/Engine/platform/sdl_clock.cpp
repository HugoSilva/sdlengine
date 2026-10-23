#include "sdl_clock.h"

#include <SDL3/SDL.h>

namespace rse::platform {
std::uint64_t SdlClock::NowMilliseconds() const { return SDL_GetTicks(); }
}  // namespace rse::platform
