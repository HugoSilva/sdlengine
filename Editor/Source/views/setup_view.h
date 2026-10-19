#pragma once

#include <core.h>

#include <SDL3/SDL.h>

#include <camera/editor_camera.h>
#include <systems/render_system.h>

class SetupView : public Scene {
 public:
  SetupView(std::string name, SDL_Window* win);

  void Update(float delta_time);
  void Render();

 private:
  EditorLayout* m_layout{nullptr};
  EditorCamera* m_camera{nullptr};
  RenderSystem* m_render;
};
