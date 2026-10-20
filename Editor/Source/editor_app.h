#pragma once

#include <core.h>

class EditorCamera;
class EditorLayout;
class RenderSystem;

// The editor's application: the window, the editor camera, the docked panels
// and the systems the editor adds to the engine's own.
//
// This used to be a Scene subclass. Scenes are data now, so what is left is
// plain application bootstrap -- the same shape as the game's main, with an
// editor UI on top.
class EditorApp : public Core {
 public:
  void Init() override;
  void Update(float delta_time) override;
  void Render() override;

  // The editor renders the scene into its viewport panel's framebuffer instead
  // of straight to the window.
  rse::Framebuffer* GetSceneTarget() override;

 private:
  EditorLayout* m_layout{nullptr};
  EditorCamera* m_camera{nullptr};
  RenderSystem* m_render{nullptr};
  rse::Shader* m_shader{nullptr};
};
