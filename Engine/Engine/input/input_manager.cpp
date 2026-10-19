#include "input_manager.h"

#include "keyboard.h"
#include "mouse.h"
#include <render/imgui_layer.h>

namespace rse {
bool InputManager::s_available_keyboard;
bool InputManager::s_available_mouse;
bool InputManager::s_available_controller;
bool InputManager::s_locked;
bool InputManager::s_quit_requested;

void InputManager::Init() {
  // TODO: check available input devices
  s_available_keyboard = true;
  s_available_mouse = true;
  s_available_controller = false;

  s_locked = false;
  s_quit_requested = false;
}

void InputManager::Update() {
  SDL_Event event;
  // Cannot be called outside of the main thread
  // Drain the whole queue: polling a single event per frame makes the UI
  // lose mouse drags during dock resizing and drops fast input.
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_EVENT_KEY_DOWN:
      case SDL_EVENT_KEY_UP:
        HandleKeyboard(event.key);
        break;
      case SDL_EVENT_MOUSE_BUTTON_DOWN:
      case SDL_EVENT_MOUSE_BUTTON_UP:
        HandleMouseButton(event.button);
        break;
      case SDL_EVENT_MOUSE_MOTION:
        HandleMouseMotion(event.motion);
        break;
      case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
      case SDL_EVENT_GAMEPAD_BUTTON_UP:
        HandleControllerButton(event.gbutton);
        break;
      case SDL_EVENT_GAMEPAD_AXIS_MOTION:
        HandleControllerAxis(event.gaxis);
        break;
      case SDL_EVENT_QUIT:
        s_quit_requested = true;
        break;
      case SDL_EVENT_MOUSE_WHEEL:
        // event.wheel.x; // Amount scrolled horizontally
        // // If negative, scrolled to the right
        // // If positive, scrolled to the left

        // event.wheel.y; // Amount scrolled vertically
        // // If negative, scrolled down
        // // If positive, scrolled up
        break;
    }
    ImGuiLayer::ProcessEvent(event);
  }
}

void InputManager::HandleKeyboard(SDL_KeyboardEvent key_event) {
  if (s_available_keyboard) {
    Keyboard::KeyCallback(key_event);
  }
}

void InputManager::HandleMouseButton(SDL_MouseButtonEvent mouse_event) {
  if (s_available_mouse) {
    Mouse::MouseButtonCallback(mouse_event);
  }
}

void InputManager::HandleMouseMotion(SDL_MouseMotionEvent mouse_event) {
  if (s_available_mouse) {
    Mouse::MousePosCallback(mouse_event);
  }
}

void InputManager::HandleControllerButton(SDL_GamepadButtonEvent event) {
  // TODO Add controller support
}

void InputManager::HandleControllerAxis(SDL_GamepadAxisEvent event) {
  // TODO Add controller support
}

const bool InputManager::IsKeyDown(int key) {
  if (s_locked) {
    return false;
  }

  return Keyboard::IsKeyDown(key);
}

const bool InputManager::IsKeyUp(int key) {
  if (s_locked) {
    return false;
  }

  return Keyboard::IsKeyUp(key);
}

const bool InputManager::IsKeyPressed(SDL_Scancode key) {
  if (s_locked) {
    return false;
  }

  return Keyboard::IsKeyPressed(key);
}
}  // namespace rse
