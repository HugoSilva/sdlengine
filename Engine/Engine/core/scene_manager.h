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

  static void Update(float delta_time);
  static void Render();

 private:
  SceneManager() {}
  static std::vector<Scene*> m_scenes;
  static unsigned int m_active_scene;
};