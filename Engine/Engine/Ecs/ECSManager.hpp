#pragma once

#include "ECS.hpp"
#include "System.hpp"
#include "ComponentDefinitions.hpp"
#include <vector>

#include <entity/registry.hpp>
#include "./Systems/BaseSystem.hpp"

#include "./Components/IncludeList.hpp"

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

		static uint32_t createEntity()
		{
			return registry.create();
		}

		static void destroyEntity(uint32_t entity)
		{
			registry.destroy(entity);
		}

		template<typename... Components, typename... TArgs>
		static void addComponent(uint32_t entity, TArgs... arguments)
		{
			registry.assign<Components...>(entity, arguments...);
		}

		template<typename Component>
		static void removeComponent(uint32_t entity)
        {
            registry.remove<Component>(entity);
		}

        template<typename... Components>
        static bool hasComponent(uint32_t entity)
        {
            return registry.has<Components...>(entity);
        }

		template<typename Component>
		static Component getComponent(uint32_t entity)
		{
			return registry.get<Component>(entity);
		}


        template<typename... Components>
        static entt::View<uint32_t, Components...> getView()
        {
            return registry.view<Components...>();
        }

		static void addSystem(SystemBase& system);
		static void addSystem(BaseSystem* system);

		static void update(float delta);
		static void clean();

		static bool save();

	private:
		static std::vector<SystemList*> m_SystemLists;
		static ECS m_ecs;
		
		static entt::DefaultRegistry registry;
		static std::vector<BaseSystem*> m_newSystemLists;
	};
}