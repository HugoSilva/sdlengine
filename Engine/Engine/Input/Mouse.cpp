#include "Mouse.hpp"

namespace rse {
double Mouse::s_x = 0;
double Mouse::s_y = 0;
// TODO: fix the initialization of the buttons
bool Mouse::s_buttons[MAX_BUTTON_SIZE] = {0};
bool Mouse::s_buttons_down[MAX_BUTTON_SIZE] = {0};
bool Mouse::s_buttons_up[MAX_BUTTON_SIZE] = {0};

double Mouse::s_last_x{0}, Mouse::s_last_y{0};
double Mouse::s_x_offset, Mouse::s_y_offset;
bool Mouse::s_first_mouse{true};

void Mouse::MousePosCallback(SDL_MouseMotionEvent event) {
  int width, height;
  SDL_GetWindowSize(SDL_GetWindowFromID(event.windowID), &width, &height);

  s_x = event.x;
  s_y = event.y;

  if (s_first_mouse) {
    s_last_x = s_x;
    s_last_y = s_y;
    s_first_mouse = false;
  }

  s_x_offset = s_x - s_last_x;
  s_y_offset = s_last_y - s_y;  // Reversed since y-coordinates go from bottom to left

  s_last_x = s_x;
  s_last_y = s_y;
}

void Mouse::MouseButtonCallback(SDL_MouseButtonEvent event) {
  Uint8 button = event.button;
  if (button < 0) return;

  if (event.down && s_buttons[button] == false) {
    s_buttons_down[button] = true;
    s_buttons_up[button] = false;
  }

  if (!event.down && s_buttons[button] == true) {
    s_buttons_down[button] = false;
    s_buttons_up[button] = true;
  }

  s_buttons[button] = event.down;
}

const bool Mouse::IsButtonDown(int button) {
  if (button < 0 || button >= MAX_BUTTON_SIZE) {
    return false;
  }
  return s_buttons_down[button];
}

const bool Mouse::IsButtonUp(int button) {
  if (button < 0 || button >= MAX_BUTTON_SIZE) {
    return false;
  }
  return s_buttons_up[button];
}

const bool Mouse::IsButtonPressed(int button) {
  if (button < 0 || button >= MAX_BUTTON_SIZE) {
    return false;
  }
  return s_buttons[button];
}
}  // namespace rse