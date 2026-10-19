#pragma once

#include <SDL3/SDL.h>

namespace rse {
#define MAX_BUTTON_SIZE 32

class Mouse {
 public:
  static void MousePosCallback(SDL_MouseMotionEvent event);
  static void MouseButtonCallback(SDL_MouseButtonEvent event);

  inline static const double GetMouseX() { return s_x_offset; }
  inline static const double GetMouseY() { return s_y_offset; }
  static const bool IsButtonDown(int button);
  static const bool IsButtonUp(int button);
  static const bool IsButtonPressed(int button);

 private:
  static double s_x;
  static double s_y;

  static bool s_buttons[];
  static bool s_buttons_down[];
  static bool s_buttons_up[];

  static double s_last_x, s_last_y;
  static double s_x_offset, s_y_offset;
  static bool s_first_mouse;
};
}  // namespace rse