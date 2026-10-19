#include "ECSManager.hpp"

#include <exception>
#include <filesystem>
#include <fstream>
#include <string>

#include <cereal/archives/xml.hpp>

#include "../Core.h"
#include "../Render/OpenGLRenderer.hpp"
#include "../Utils/Logger.hpp"
#include "./Components/IncludeList.hpp"
#include "./Systems/AudioSystem.hpp"
#include "./Systems/BaseSystem.hpp"
#include "./Systems/PhysicsSystem.hpp"
#include "./Systems/SpriteRenderSystem.hpp"

namespace {
constexpr const char* kProjectFile{"project.xml"};

// The project is written to a scratch file and moved into place once the
// archive has closed cleanly, so a failed save cannot destroy the previous
// (loadable) project file.
constexpr const char* kProjectFileScratch{"project.xml.tmp"};
}  // namespace

namespace rse {
entt::DefaultRegistry ECSManager::registry;
std::vector<BaseSystem*> ECSManager::m_systems_list;
std::vector<BaseSystem*> ECSManager::m_pre_scene_systems;

void ECSManager::Init() {
  AudioSystem* audio_system = new AudioSystem();
  AddSystem(audio_system);
  PhysicsSystem* physics_system = new PhysicsSystem();
  AddSystem(physics_system);
  SpriteRenderSystem* sprite_system = new SpriteRenderSystem(Core::GetRenderer());
  AddSystem(sprite_system);
}

void ECSManager::AddSystem(BaseSystem* system, SystemPhase phase) {
  if (phase == SystemPhase::kPreScene) {
    m_pre_scene_systems.push_back(system);
    return;
  }

  m_systems_list.push_back(system);
}

void ECSManager::Update(float delta) {
  for (unsigned int i = 0; i < m_pre_scene_systems.size(); i++) m_pre_scene_systems[i]->Update(delta);

  for (unsigned int i = 0; i < m_systems_list.size(); i++) m_systems_list[i]->Update(delta);
}

void ECSManager::UpdateUi() {
  // Only the editor's RenderSystem implements renderUI, so the order of the
  // two phases makes no difference here; walking them the same way as
  // update() just keeps the two functions from disagreeing.
  for (unsigned int i = 0; i < m_pre_scene_systems.size(); i++) m_pre_scene_systems[i]->RenderUi();

  for (unsigned int i = 0; i < m_systems_list.size(); i++) m_systems_list[i]->RenderUi();
}

void ECSManager::Clean() {
  for (unsigned int i = 0; i < m_pre_scene_systems.size(); i++) delete m_pre_scene_systems[i];
  m_pre_scene_systems.clear();

  for (unsigned int i = 0; i < m_systems_list.size(); i++) delete m_systems_list[i];
  m_systems_list.clear();
}

bool ECSManager::Save() {
  std::ofstream os(kProjectFileScratch);

  if (!os.is_open()) {
    Logger::Error("Failed to save the project: project.xml could not be opened for writing.");
    return false;
  }

  bool succeeded = true;

  try {
    cereal::XMLOutputArchive output{os};

    registry.snapshot().entities(output).component<BasicComponent, TransformComponent, SpriteComponent>(output);
  } catch (const std::exception& error) {
    Logger::Error(std::string("Failed to save the project: ") + error.what());
    succeeded = false;
  } catch (...) {
    Logger::Error("Failed to save the project: unknown error.");
    succeeded = false;
  }

  os.close();

  std::error_code error;

  if (!succeeded) {
    std::filesystem::remove(kProjectFileScratch, error);
    return false;
  }

  std::filesystem::rename(kProjectFileScratch, kProjectFile, error);

  if (error) {
    Logger::Error(std::string("Failed to save the project: ") + error.message());
    std::filesystem::remove(kProjectFileScratch, error);
    return false;
  }

  Logger::Log("Project saved to project.xml");
  return true;
}

bool ECSManager::Load() {
  std::ifstream is(kProjectFile);

  if (!is.is_open()) {
    Logger::Error("Failed to load the project: project.xml was not found.");
    return false;
  }

  try {
    cereal::XMLInputArchive input{is};

    registry.restore().entities(input).component<BasicComponent, TransformComponent, SpriteComponent>(input);
  } catch (const std::exception& error) {
    Logger::Error(std::string("Failed to load the project: ") + error.what());
    return false;
  } catch (...) {
    Logger::Error("Failed to load the project: unknown error.");
    return false;
  }

  Logger::Log("Project loaded from project.xml");
  return true;
}
}  // namespace rse