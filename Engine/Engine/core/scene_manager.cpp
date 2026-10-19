#include "scene_manager.h"

#include "scene.h"

std::vector<Scene*> SceneManager::m_scenes;
unsigned int SceneManager::m_active_scene{0};

void SceneManager::Init() {}

void SceneManager::Add(Scene* scene) { m_scenes.push_back(scene); }

Scene* SceneManager::Get(const std::string& name) {
  for (Scene* scene : m_scenes) {
    if (scene->GetName() == name) return scene;
  }
  return nullptr;
}

bool SceneManager::Change(const std::string& name) {
  for (unsigned int i = 0; i < m_scenes.size(); i++) {
    if (m_scenes.at(i)->GetName() == name) {
      m_active_scene = i;
      return true;
    }
  }
  return false;
}

void SceneManager::Clean() {
  for (unsigned int i = 0; i < m_scenes.size(); i++) delete m_scenes[i];
}

void SceneManager::Update(float delta_time) {
  if (m_scenes.size() > 0) {
    m_scenes[m_active_scene]->Update(delta_time);
  }
}

void SceneManager::Render() {
  if (m_scenes.size() > 0) {
    m_scenes[m_active_scene]->Render();
  }
}