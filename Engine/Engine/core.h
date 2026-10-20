#pragma once

// Needed for the entt lib
#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifdef EMSCRIPTEN
#include <emscripten/emscripten.h>
#endif

#include <cstdint>
#include <vector>

#include <glm/glm.hpp>
#if _WIN32
#include <Windows.h>
#endif

#include <audio/music.h>
#include <audio/sound.h>
#include <audio/sound_manager.h>
#include <components/sprite.h>
#include <core/event_manager.h>
#include <core/scene_manager.h>
#include <core/texture_manager.h>
#include <core/thread_manager.h>
#include <ecs/components/include_list.h>
#include <ecs/ecs_manager.h>
#include <ecs/systems/base_system.h>
#include <input/input_manager.h>
#include <physics/collider.h>
#include <physics/physics_manager.h>
#include <physics/rigidbody.h>
#include <render/camera.h>
#include <render/font.h>
#include <render/font_manager.h>
#include <render/framebuffer.h>
#include <render/layer.h>
#include <render/opengl_renderer.h>
#include <render/renderable_2d.h>
#include <render/renderer_2d.h>
#include <render/window.h>
#include <utils/file.h>

class Core {
 public:
  void Start();
  static bool GetRunning() { return m_running; }
  static rse::Renderer2D* GetRenderer() { return m_renderer; }
  static rse::Window* GetWindow() { return m_window; }

  // Render target the scene pass should draw into, or nullptr to draw into the
  // window's back buffer. The editor overrides this so the scene lands in its
  // viewport panel instead of the whole window.
  virtual rse::Framebuffer* GetSceneTarget() { return nullptr; }

 protected:
  Core();
  virtual ~Core();

  rse::Window* createWindow(const char* name, int width, int height);

  virtual void Init() = 0;
  virtual void Tick() {}

  // Runs once a frame, before the UI and the scene pass. Per-frame behaviour
  // belongs here (or in an ECS system): a scene is data, not a class.
  virtual void Update(float delta_time) {}

  // Runs after the render target is bound and the UI phase is over, so a
  // shader can upload the uniforms the camera ended up with this frame.
  virtual void Render() = 0;

  const unsigned int GetFps() const { return m_frames_per_second; }

 private:
  void Run();

  // SDL3's SDL_GetTicks() returns Uint64 (milliseconds).
  uint64_t m_now = 0, m_last = 0;
  float m_delta_time = 0.0;
  float m_delta_accumulator = 0.0f;
  int m_frames = 0;
  unsigned int m_frames_per_second;
  static bool m_running;
  static rse::Renderer2D* m_renderer;
  static rse::Window* m_window;
};