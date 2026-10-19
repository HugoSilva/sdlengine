#pragma once

#include <SDL3/SDL.h>

namespace rse {
class InputManager {
 public:
  static void Init();
  static void Update();

  static const bool IsKeyDown(int key);
  static const bool IsKeyUp(int key);
  static const bool IsKeyPressed(SDL_Scancode key);
  inline static const void Lock() { s_locked = true; }
  inline static const void Unlock() { s_locked = false; }

  inline static const bool IsQuitRequested() { return s_quit_requested; }

 private:
  InputManager() {}

  static void HandleKeyboard(SDL_KeyboardEvent window_event);
  static void HandleMouseButton(SDL_MouseButtonEvent window_event);
  static void HandleMouseMotion(SDL_MouseMotionEvent window_event);
  static void HandleControllerButton(SDL_GamepadButtonEvent event);
  static void HandleControllerAxis(SDL_GamepadAxisEvent event);

  static bool s_available_keyboard;
  static bool s_available_mouse;
  static bool s_available_controller;
  static bool s_locked;
  static bool s_quit_requested;
};
}  // namespace rse