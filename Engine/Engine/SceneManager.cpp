#include "SceneManager.h"

std::vector<Scene*> SceneManager::m_Scenes;

void SceneManager::init()
{
}

void SceneManager::add(Scene* scene)
{
	m_Scenes.push_back(scene);
}

Scene* SceneManager::get(const std::string& name)
{
	for (Scene* scene : m_Scenes)
	{
		if (scene->getName() == name)
			return scene;
	}
	return nullptr;
}

void SceneManager::clean()
{
	for (unsigned int i = 0; i < m_Scenes.size(); i++)
		delete m_Scenes[i];
}