#include "editor_app.h"

#include <editor_manager.h>

#include <ecs/scene_io.h>
#include <layout/editor_layout.h>
#include <panels/console_panel.h>
#include <panels/content_browser_panel.h>
#include <panels/debug_panel.h>
#include <panels/hierarchy_panel.h>
#include <panels/inspector_panel.h>
#include <panels/viewport_panel.h>
#include <render/grid_render_system.h>
#include <systems/render_system.h>
#include <utils/logger.h>

void EditorApp::Init() {
  using namespace rse;

  m_shader = new Shader("Resources/Default.vert", "Resources/Default.frag");

  // TODO: load the project's own resources. The manifest's asset root is not
  // consulted yet; see the project format document's follow-ups.
  TextureManager::Add(new Texture("test00", "Resources/tex3.png"));
  SoundManager::Add(new Sound("eff", "Resources/effect.wav"));

  // The viewport gets the editor's own camera, not the engine's game camera:
  // the engine camera keeps its WASD/mouse-look path and the editor camera
  // owns pan/zoom and the projection the viewport draws with.
  m_camera = new EditorCamera();

  m_layout = new EditorLayout();
  m_layout->AddPanel(std::make_unique<HierarchyPanel>());
  m_layout->AddPanel(std::make_unique<ContentBrowserPanel>());
  m_layout->AddPanel(std::make_unique<DebugPanel>());
  m_layout->AddPanel(std::make_unique<ViewportPanel>(m_camera));
  m_layout->AddPanel(std::make_unique<InspectorPanel>());
  m_layout->AddPanel(std::make_unique<ConsolePanel>());

  m_render = new RenderSystem(m_layout);
  ECSManager::AddSystem(m_render);

  // The grid is submitted into the scene pass before the sprites are, which is
  // what puts it underneath them.
  ECSManager::AddSystem(new GridRenderSystem(Core::GetRenderer(), m_camera), rse::SystemPhase::kPreScene);

  EditorManager::Init();

#ifdef EMSCRIPTEN
#else
  ThreadManager::Init();
  ThreadManager::Start();
#endif
}

void EditorApp::Update(float delta_time) {
  // Temporary: scenes have no project yet, so the keys still read and write
  // the old fixed file. The project work replaces both with the scene the
  // project has open.
  if (rse::InputManager::IsKeyPressed(SDL_SCANCODE_F1)) {
    std::string error;
    if (!rse::SceneIO::Save("project.xml", &error)) {
      Logger::Error("Failed to save the scene: " + error);
    }
  }
  if (rse::InputManager::IsKeyPressed(SDL_SCANCODE_F2)) {
    std::string error;
    if (!rse::SceneIO::Load("project.xml", &error)) {
      Logger::Error("Failed to load the scene: " + error);
    }
  }
}

void EditorApp::Render() {
  m_shader->Enable();
  m_camera->Apply(m_shader);
}

rse::Framebuffer* EditorApp::GetSceneTarget() { return EditorManager::GetSceneTarget(); }
