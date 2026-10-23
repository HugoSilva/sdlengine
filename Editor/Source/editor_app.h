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
  explicit EditorApp(const std::filesystem::path& project_file);

  void Init() override;
  void Update(float delta_time) override;
  void Render() override;

  // Writes the session back to editor.cfg: which project and scene were open,
  // and which panels were visible.
  void Shutdown() override;

  // The editor renders the scene into its viewport panel's framebuffer instead
  // of straight to the window.
  rse::Framebuffer* GetSceneTarget() override;

  // Scene logic only runs while the open scene is playing; a game always runs
  // it.
  bool SceneLogicEnabled() const override;

 private:
  void UpdateTitle();

  // Records the open project and scene, saving only when one of them changed.
  void RememberSession();

  EditorLayout* m_layout{nullptr};
  EditorCamera* m_camera{nullptr};
  RenderSystem* m_render{nullptr};
  rse::Shader* m_shader{nullptr};
  std::string m_title;
  // The manifest the panels were last told about; empty while no project is
  // open. See Update.
  std::filesystem::path m_last_project_file;
};
