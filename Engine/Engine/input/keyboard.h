#pragma once

#include <SDL3/SDL.h>

namespace rse {
// What the keyboard is doing, in the three states a frame cares about.
//
// A frame reads input once, after InputManager::Update has drained the event
// queue, so "pressed" and "released" describe what happened since the previous
// frame began -- not whether the key is down now. Holding a key for a second is
// one press, and the queries keep saying the key is down; a game that wants a
// repeat counts frames or time, because SDL's own key repeats arrive as events
// and are ignored here.
class Keyboard {
 public:
  static void KeyCallback(SDL_KeyboardEvent event);

  // Clears the press and release latches. InputManager calls it once, before
  // draining the queue, which is what makes those latches last exactly one
  // frame.
  static void BeginFrame();

  // The key is down right now.
  static bool IsKeyDown(int key);

  // The key went down since the frame began. A key still held from an earlier
  // frame is down, not pressed.
  static bool IsKeyPressed(int key);

  // The key came up since the frame began.
  static bool IsKeyReleased(int key);

 private:
  // Held right now: SDL sends a key-up when the key goes up and nothing else
  // changes it, so this is the key's state rather than an edge.
  static bool s_keys[];

  // Edges, cleared by BeginFrame.
  static bool s_pressed[];
  static bool s_released[];
};
}  // namespace rse
