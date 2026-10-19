#include "keyboard.h"

namespace rse {
bool Keyboard::s_keys[SDL_SCANCODE_COUNT] = {0};
bool Keyboard::s_keys_down[SDL_SCANCODE_COUNT] = {0};
bool Keyboard::s_keys_up[SDL_SCANCODE_COUNT] = {0};

void Keyboard::KeyCallback(SDL_KeyboardEvent event) {
  SDL_Scancode key = event.scancode;
  if (key < 0) return;

  if (event.down && s_keys[key] == false) {
    s_keys_down[key] = true;
    s_keys_up[key] = false;
  }

  if (!event.down && s_keys[key] == true) {
    s_keys_down[key] = false;
    s_keys_up[key] = true;
  }

  s_keys[key] = event.down;
}

const bool Keyboard::IsKeyDown(int key) {
  if (key < 0 || key >= SDL_SCANCODE_COUNT) {
    return false;
  }
  return s_keys_down[key];
}

const bool Keyboard::IsKeyUp(int key) {
  if (key < 0 || key >= SDL_SCANCODE_COUNT) {
    return false;
  }
  return s_keys_up[key];
}

const bool Keyboard::IsKeyPressed(int key) {
  if (key < 0 || key >= SDL_SCANCODE_COUNT) {
    return false;
  }
  return s_keys[key];
}
}  // namespace rse
