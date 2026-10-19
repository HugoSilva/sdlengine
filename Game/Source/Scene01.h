#pragma once

#include <Core.h>

#include <SDL3/SDL.h>

using namespace rse;

class SpriteTest : public Scene {
 public:
  SpriteTest(SDL_Window* win);
  ~SpriteTest();

  void Update(float delta_time);
  void Render();

 private:
  Layer* m_layer;
  // Label* m_Fps;
  Sprite* m_ground_sprite;
  Sprite* m_player_sprite;
  Shader* m_shader;

  Camera* m_camera;
};
