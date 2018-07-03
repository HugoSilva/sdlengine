#include "SceneManager.h"

std::vector<Scene*> SceneManager::m_Scenes;
unsigned int SceneManager::m_ActiveScene{ 0 };

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

bool SceneManager::change(const std::string& name)
{
	for (unsigned int i = 0; i < m_Scenes.size(); i++)
	{
		if (m_Scenes.at(i)->getName() == name)
		{
			m_ActiveScene = i;
			return true;
		}
	}
	return false;
}

void SceneManager::clean()
{
	for (unsigned int i = 0; i < m_Scenes.size(); i++)
		delete m_Scenes[i];
}

void SceneManager::update(float deltaTime)
{
	if (m_Scenes.size() > 0)
	{
		m_Scenes[m_ActiveScene]->Update(deltaTime);
	}
}

void SceneManager::render()
{
	if (m_Scenes.size() > 0)
	{
		m_Scenes[m_ActiveScene]->Render();
	}
}