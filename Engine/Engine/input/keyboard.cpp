#include "keyboard.h"

#include <algorithm>
#include <iterator>

namespace rse {
bool Keyboard::s_keys[SDL_SCANCODE_COUNT] = {0};
bool Keyboard::s_pressed[SDL_SCANCODE_COUNT] = {0};
bool Keyboard::s_released[SDL_SCANCODE_COUNT] = {0};

namespace {
bool InRange(int key) { return key >= 0 && key < SDL_SCANCODE_COUNT; }
}  // namespace

void Keyboard::KeyCallback(SDL_KeyboardEvent event) {
  const int key = event.scancode;

  if (!InRange(key)) {
    return;
  }

  if (event.down) {
    // SDL reports a held key again and again (its own repeat); only the first
    // event of a hold is a press.
    if (!s_keys[key]) {
      s_pressed[key] = true;
    }

    s_keys[key] = true;
    return;
  }

  // A key-up for a key this system never saw go down (the window lost focus and
  // the release happened elsewhere) is not a release of anything.
  if (s_keys[key]) {
    s_released[key] = true;
  }

  s_keys[key] = false;
}

void Keyboard::BeginFrame() {
  // Only the edges: a key that is still down is still down.
  std::fill(std::begin(s_pressed), std::end(s_pressed), false);
  std::fill(std::begin(s_released), std::end(s_released), false);
}

bool Keyboard::IsKeyDown(int key) {
  if (!InRange(key)) {
    return false;
  }

  return s_keys[key];
}

bool Keyboard::IsKeyPressed(int key) {
  if (!InRange(key)) {
    return false;
  }

  return s_pressed[key];
}

bool Keyboard::IsKeyReleased(int key) {
  if (!InRange(key)) {
    return false;
  }

  return s_released[key];
}
}  // namespace rse
