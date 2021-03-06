#include "ECSManager.hpp"

#include <cereal/archives/xml.hpp>
#include <fstream>

#include "../Core.h"
#include "../Render/OpenGLRenderer.hpp"
#include "./Components/IncludeList.hpp"
#include "./Systems/AudioSystem.hpp"
#include "./Systems/BaseSystem.hpp"
#include "./Systems/PhysicsSystem.hpp"
#include "./Systems/SpriteRenderSystem.hpp"

namespace rse
{
	entt::DefaultRegistry ECSManager::registry;
	entt::DefaultRegistry ECSManager::editorRegistry;
	std::vector<BaseSystem*> ECSManager::m_systemsList;

	void ECSManager::init()
	{
		AudioSystem* audioSystem = new AudioSystem();
		addSystem(audioSystem);
		PhysicsSystem* physicsSystem = new PhysicsSystem();
		addSystem(physicsSystem);
		SpriteRenderSystem* spriteSystem = new SpriteRenderSystem(Core::getRenderer());
		addSystem(spriteSystem);
	}

	void ECSManager::addSystem(BaseSystem* system)
	{
		m_systemsList.push_back(system);
	}

	void ECSManager::update(float delta)
	{
		for (unsigned int i = 0; i < m_systemsList.size(); i++)
			m_systemsList[i]->update(delta);
	}

	void ECSManager::clean()
	{
		for (unsigned int i = 0; i < m_systemsList.size(); i++)
			delete m_systemsList[i];
	}

	bool ECSManager::save()
	{
		std::ofstream os("project.xml");
		cereal::XMLOutputArchive output{os};

		//std::ofstream os("project.rse", std::ios::binary);
		//cereal::BinaryOutputArchive archive(os);
		
		registry.snapshot()
			.entities(output)
			.component<BasicComponent, TransformComponent, SpriteComponent>(output);

		return true;
	}

	bool ECSManager::load()
	{
		std::ifstream os("project.xml");
		cereal::XMLInputArchive input{os};

		registry.restore()
			.entities(input)
			.component<BasicComponent, TransformComponent, SpriteComponent>(input);

		return true;
	}
}