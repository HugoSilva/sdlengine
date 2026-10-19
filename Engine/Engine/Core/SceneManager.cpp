#include "SceneManager.hpp"

#include "Scene.hpp"

std::vector<Scene*> SceneManager::m_Scenes;
unsigned int SceneManager::m_ActiveScene{0};

void SceneManager::Init() {}

void SceneManager::Add(Scene* scene) { m_Scenes.push_back(scene); }

Scene* SceneManager::Get(const std::string& name) {
  for (Scene* scene : m_Scenes) {
    if (scene->GetName() == name) return scene;
  }
  return nullptr;
}

bool SceneManager::Change(const std::string& name) {
  for (unsigned int i = 0; i < m_Scenes.size(); i++) {
    if (m_Scenes.at(i)->GetName() == name) {
      m_ActiveScene = i;
      return true;
    }
  }
  return false;
}

void SceneManager::Clean() {
  for (unsigned int i = 0; i < m_Scenes.size(); i++) delete m_Scenes[i];
}

void SceneManager::Update(float deltaTime) {
  if (m_Scenes.size() > 0) {
    m_Scenes[m_ActiveScene]->Update(deltaTime);
  }
}

void SceneManager::Render() {
  if (m_Scenes.size() > 0) {
    m_Scenes[m_ActiveScene]->Render();
  }
}