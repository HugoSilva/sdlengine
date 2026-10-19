#include "InputManager.hpp"

#include "../Render/ImGuiLayer.hpp"
#include "Keyboard.hpp"
#include "Mouse.hpp"

namespace rse {
bool InputManager::s_AvailableKeyboard;
bool InputManager::s_AvailableMouse;
bool InputManager::s_AvailableController;
bool InputManager::s_Locked;
bool InputManager::s_QuitRequested;

void InputManager::Init() {
  // TODO: check available input devices
  s_AvailableKeyboard = true;
  s_AvailableMouse = true;
  s_AvailableController = false;

  s_Locked = false;
  s_QuitRequested = false;
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
        s_QuitRequested = true;
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

void InputManager::HandleKeyboard(SDL_KeyboardEvent keyEvent) {
  if (s_AvailableKeyboard) {
    Keyboard::KeyCallback(keyEvent);
  }
}

void InputManager::HandleMouseButton(SDL_MouseButtonEvent mouseEvent) {
  if (s_AvailableMouse) {
    Mouse::MouseButtonCallback(mouseEvent);
  }
}

void InputManager::HandleMouseMotion(SDL_MouseMotionEvent mouseEvent) {
  if (s_AvailableMouse) {
    Mouse::MousePosCallback(mouseEvent);
  }
}

void InputManager::HandleControllerButton(SDL_GamepadButtonEvent event) {
  // TODO Add controller support
}

void InputManager::HandleControllerAxis(SDL_GamepadAxisEvent event) {
  // TODO Add controller support
}

const bool InputManager::IsKeyDown(int key) {
  if (s_Locked) {
    return false;
  }

  return Keyboard::IsKeyDown(key);
}

const bool InputManager::IsKeyUp(int key) {
  if (s_Locked) {
    return false;
  }

  return Keyboard::IsKeyUp(key);
}

const bool InputManager::IsKeyPressed(SDL_Scancode key) {
  if (s_Locked) {
    return false;
  }

  return Keyboard::IsKeyPressed(key);
}
}  // namespace rse
