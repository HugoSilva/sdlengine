#include "ecs_manager.h"

#include <core.h>

#include <ecs/components/include_list.h>
#include <ecs/systems/audio_system.h>
#include <ecs/systems/base_system.h>
#include <ecs/systems/physics_system.h>
#include <ecs/systems/sprite_render_system.h>
#include <render/opengl_renderer.h>

namespace rse {
entt::DefaultRegistry ECSManager::registry;
std::vector<BaseSystem*> ECSManager::m_systems_list;
std::vector<BaseSystem*> ECSManager::m_pre_scene_systems;
std::vector<BaseSystem*> ECSManager::m_simulation_systems;

void ECSManager::Init() {
  AudioSystem* audio_system = new AudioSystem();
  AddSystem(audio_system, SystemPhase::kSimulation);
  PhysicsSystem* physics_system = new PhysicsSystem();
  AddSystem(physics_system, SystemPhase::kSimulation);
  SpriteRenderSystem* sprite_system = new SpriteRenderSystem(Core::GetRenderer());
  AddSystem(sprite_system);
}

void ECSManager::AddSystem(BaseSystem* system, SystemPhase phase) {
  if (phase == SystemPhase::kPreScene) {
    m_pre_scene_systems.push_back(system);
    return;
  }

  if (phase == SystemPhase::kSimulation) {
    m_simulation_systems.push_back(system);
    return;
  }

  m_systems_list.push_back(system);
}

void ECSManager::Update(float delta, bool run_simulation) {
  for (unsigned int i = 0; i < m_pre_scene_systems.size(); i++) m_pre_scene_systems[i]->Update(delta);

  if (run_simulation) {
    for (unsigned int i = 0; i < m_simulation_systems.size(); i++) m_simulation_systems[i]->Update(delta);
  }

  for (unsigned int i = 0; i < m_systems_list.size(); i++) m_systems_list[i]->Update(delta);
}

void ECSManager::UpdateUi() {
  // Only the editor's RenderSystem implements renderUI, so the order of the
  // phases makes no difference here; walking them the same way as Update()
  // just keeps the two functions from disagreeing. The simulation phase is
  // walked too: a system that draws settings for its own logic still has UI
  // while the scene is stopped.
  for (unsigned int i = 0; i < m_pre_scene_systems.size(); i++) m_pre_scene_systems[i]->RenderUi();

  for (unsigned int i = 0; i < m_simulation_systems.size(); i++) m_simulation_systems[i]->RenderUi();

  for (unsigned int i = 0; i < m_systems_list.size(); i++) m_systems_list[i]->RenderUi();
}

void ECSManager::Clean() {
  for (unsigned int i = 0; i < m_pre_scene_systems.size(); i++) delete m_pre_scene_systems[i];
  m_pre_scene_systems.clear();

  for (unsigned int i = 0; i < m_simulation_systems.size(); i++) delete m_simulation_systems[i];
  m_simulation_systems.clear();

  for (unsigned int i = 0; i < m_systems_list.size(); i++) delete m_systems_list[i];
  m_systems_list.clear();
}
}  // namespace rse