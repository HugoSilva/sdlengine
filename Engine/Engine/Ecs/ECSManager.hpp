#pragma once

#include "ECS.hpp"
#include "System.hpp"
#include "ComponentDefinitions.hpp"
#include <vector>

#include <entity/registry.hpp>

namespace ecs
{
	class ECSManager
	{
	public:
		ECSManager() { }

		static void init();

		template<class... TArgs>
		static EntityHandle addEntity(TArgs... arguments)
		{
			return m_ecs.makeEntity((arguments)...);

			auto entity = registry.create();
			registry.destroy(entity);
		}

		uint32_t createEntity()
		{
			return registry.create();
		}

		void destroyEntity(uint32_t entity)
		{
			registry.destroy(entity);
		}

		static void addSystem(SystemBase& system);

		static void update(float delta);
		static void clean();

		static bool save();

		static std::vector<int> getCurrentEntities();

	private:
		static std::vector<SystemList*> m_SystemLists;
		static ECS m_ecs;
		
		entt::DefaultRegistry registry;
	};
}