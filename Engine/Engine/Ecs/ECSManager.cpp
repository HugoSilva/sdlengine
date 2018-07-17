#include "ECSManager.hpp"
#include "ComponentDefinitions.hpp"

namespace ecs
{
	std::vector<SystemList*> ECSManager::m_SystemLists;
	ECS ECSManager::m_ecs;

	void ECSManager::init()
	{
		//TODO Search if there is a better place to create the default SystemList or lists
		ecs::SystemList* mainSystems;
		m_SystemLists.push_back(mainSystems);

		SpriteRenderSystem spriteSystem;
		addSystem(spriteSystem);
	}

	template<class... TArgs>
	void ECSManager::addEntity(TArgs&&... arguments)
	{
		m_ecs.makeEntity((&arguments)...);
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