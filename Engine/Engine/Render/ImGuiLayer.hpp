#pragma once

#include <SDL3/SDL.h>

namespace rse {
// Owns the Dear ImGui context plus the SDL3/OpenGL3 backends and drives the
// ImGui frame lifecycle.
//
// This is deliberately kept out of Window and OpenGLRenderer so that the
// scene pass and the UI pass are two independent steps of Core::run():
//
//     ImGuiLayer::beginFrame();                     // ImGui::NewFrame
//     ... build the UI (any ImGui:: call is legal from here on) ...
//     renderer->begin(); ...; renderer->flush();    // scene pass only
//     ImGuiLayer::endFrame();                       // ImGui::Render + present
//
// The renderer used to do the two ImGui calls plus the buffer swap itself,
// which made it impossible to render the scene into a texture first.
class ImGuiLayer {
 public:
  static void init(SDL_Window* window, SDL_GLContext context);
  static void shutdown();

  static void beginFrame();
  static void endFrame();

  // Forward a platform event to the SDL3 backend.
  static void processEvent(const SDL_Event& event);
};
}  // namespace rse
