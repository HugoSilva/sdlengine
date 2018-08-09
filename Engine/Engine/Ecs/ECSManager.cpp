#include "ECSManager.hpp"
#include "ComponentDefinitions.hpp"
#include "../Graphics/OpenGLRenderer.h"
#include "../Core.h"
#include <cereal/archives/xml.hpp>
#include <fstream>

namespace ecs
{
	std::vector<SystemList*> ECSManager::m_SystemLists;
	ECS ECSManager::m_ecs;
	entt::DefaultRegistry ECSManager::registry;

	void ECSManager::init()
	{
		//TODO Search if there is a better place to create the default SystemList or lists
		ecs::SystemList* mainSystems = new ecs::SystemList();
		m_SystemLists.push_back(mainSystems);
		//TODO create static method to fetch the window
		SpriteRenderSystem* spriteSystem = new SpriteRenderSystem(*Core::getRenderer());
		addSystem(*spriteSystem);
		DebugRenderSystem* debugSystem = new DebugRenderSystem(*Core::getRenderer());
		addSystem(*debugSystem);
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

	bool ECSManager::save()
	{
		std::ofstream os("project.xml");
		cereal::XMLOutputArchive archive(os);

		//std::ofstream os("project.rse", std::ios::binary);
		//cereal::BinaryOutputArchive archive(os);
		
		archive(cereal::make_nvp("scene", m_ecs));

		return true;
	}

	std::vector<int> ECSManager::getCurrentEntities()
	{
		return m_ecs.getEntites();
	}
}