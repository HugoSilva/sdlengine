#pragma once

#include <vector>

class Scene; //forward declaration for Scene

class SceneManager
{
public:
	static void init();
	static void add(Scene* scene);
	static Scene* get(const std::string& name);
	static bool change(const std::string& name);
	static void clean();

	static void update(float deltaTime);
	static void render();

private:
	SceneManager() { }
	static std::vector<Scene*> m_Scenes;
	static unsigned int m_ActiveScene;
};