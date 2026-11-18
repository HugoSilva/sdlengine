#include "ecs_manager.h"

#include <core.h>

#include <ecs/components/include_list.h>
#include <ecs/entity.h>
#include <ecs/scene.h>
#include <ecs/systems/audio_system.h>
#include <ecs/systems/base_system.h>
#include <ecs/systems/physics_system.h>
#include <ecs/systems/sprite_render_system.h>

namespace rse {
Scene ECSManager::m_scene;
std::vector<BaseSystem*> ECSManager::m_systems_list;
std::vector<BaseSystem*> ECSManager::m_pre_scene_systems;
std::vector<BaseSystem*> ECSManager::m_simulation_systems;

Scene& ECSManager::ActiveScene() { return m_scene; }

entt::DefaultRegistry& ECSManager::Registry() { return m_scene.m_registry; }

void ECSManager::Init() {
  AudioSystem* audio_system = new AudioSystem();
  AddSystem(audio_system, SystemPhase::kSimulation);
  PhysicsSystem* physics_system = new PhysicsSystem();
  AddSystem(physics_system, SystemPhase::kSimulation);
  SpriteRenderSystem* sprite_system = new SpriteRenderSystem(Core::GetRenderer());
  AddSystem(sprite_system);
}

uint32_t ECSManager::CreateEntity() { return m_scene.CreateEntity().Handle(); }

void ECSManager::SetEntityEnabled(uint32_t entity, bool enabled) {
  if (!HasComponent<EntityEnabledComponent>(entity)) {
    AddComponent<EntityEnabledComponent>(entity, enabled);
    return;
  }

  GetComponent<EntityEnabledComponent>(entity).m_enabled = enabled;
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

void ECSManager::Update(float alpha) {
  for (unsigned int i = 0; i < m_pre_scene_systems.size(); i++) m_pre_scene_systems[i]->Update(m_scene, alpha);

  for (unsigned int i = 0; i < m_systems_list.size(); i++) m_systems_list[i]->Update(m_scene, alpha);
}

void ECSManager::UpdateSimulation(float step) {
  for (unsigned int i = 0; i < m_simulation_systems.size(); i++)
    m_simulation_systems[i]->UpdateSimulation(m_scene, step);
}

void ECSManager::UpdateUi() {
  // Only the editor's RenderSystem implements renderUI, so the order of the
  // phases makes no difference here; walking them the same way as Update()
  // just keeps the two functions from disagreeing. The simulation phase is
  // walked too: a system that draws settings for its own logic still has UI
  // while the scene is stopped.
  for (unsigned int i = 0; i < m_pre_scene_systems.size(); i++) m_pre_scene_systems[i]->RenderUi(m_scene);

  for (unsigned int i = 0; i < m_simulation_systems.size(); i++) m_simulation_systems[i]->RenderUi(m_scene);

  for (unsigned int i = 0; i < m_systems_list.size(); i++) m_systems_list[i]->RenderUi(m_scene);
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