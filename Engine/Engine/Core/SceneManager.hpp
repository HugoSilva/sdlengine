#pragma once

#include <string>
#include <vector>

class Scene;  // forward declaration for Scene

class SceneManager {
 public:
  static void Init();
  static void Add(Scene* scene);
  static Scene* Get(const std::string& name);
  static bool Change(const std::string& name);
  static void Clean();

  static void Update(float deltaTime);
  static void Render();

 private:
  SceneManager() {}
  static std::vector<Scene*> m_Scenes;
  static unsigned int m_ActiveScene;
};