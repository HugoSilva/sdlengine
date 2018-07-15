#include "ECSManager.hpp"

namespace ecs
{
	ECS ECSManager::m_ecs;

	void ECSManager::init()
	{
		//TODO need to review the static functions for the manager
		//ecs::ECSManager ecs;
		//ecs::SystemList mainSystems;
		//mainSystems.addSystem(newSystem);
	}

	void ECSManager::addSystemList()
	{
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