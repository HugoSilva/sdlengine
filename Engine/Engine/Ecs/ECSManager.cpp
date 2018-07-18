#include "ECSManager.hpp"
#include "ComponentDefinitions.hpp"
#include "../Graphics/OpenGLRenderer.h"
#include "../Core.h"

namespace ecs
{
	std::vector<SystemList*> ECSManager::m_SystemLists;
	ECS ECSManager::m_ecs;

	void ECSManager::init()
	{
		//TODO Search if there is a better place to create the default SystemList or lists
		ecs::SystemList* mainSystems = new ecs::SystemList();
		m_SystemLists.push_back(mainSystems);
		//TODO create static method to fetch the window
		SpriteRenderSystem* spriteSystem = new SpriteRenderSystem(*Core::getRenderer());
		addSystem(*spriteSystem);
	}

	//TODO Fix static type casting to variadic template
	void ECSManager::addEntity(PositionComponent a, SpriteComponent b)
	{
		m_ecs.makeEntity(a, b);
	}

	void ECSManager::addSystem(SystemBase& system)
	{
		m_SystemLists[0]->addSystem(system);
	}

	void ECSManager::update(float delta)
	{
		for (unsigned int i = 0; i < m_SystemLists.size(); i++)
			m_ecs.updateSystems(*m_SystemLists[i], delta);
	}

	void ECSManager::clean()
	{
		for (unsigned int i = 0; i < m_SystemLists.size(); i++)
			delete m_SystemLists[i];
	}
}