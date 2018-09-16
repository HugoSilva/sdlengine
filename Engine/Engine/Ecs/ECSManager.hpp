#pragma once

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

		static uint32_t createEntity()
		{
			return registry.create();
		}

		static uint32_t createEditorEntity()
		{
			return editorRegistry.create();
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

		template<typename... Components, typename... TArgs>
		static void addEditorComponent(uint32_t entity, TArgs... arguments)
		{
			editorRegistry.assign<Components...>(entity, arguments...);
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
		static Component& getComponent(uint32_t entity)
		{
			return registry.get<Component>(entity);
		}
		
		template<typename... Components>
        static entt::View<uint32_t, Components...> getView()
        {
            return registry.view<Components...>();
        }

		template<typename... Components>
		static entt::View<uint32_t, Components...> getEditorView()
		{
			return editorRegistry.view<Components...>();
		}

		static void addSystem(BaseSystem* system);

		static void update(float delta);
		static void clean();

		static bool save();
		static bool load();

	private:
		static entt::DefaultRegistry registry;
		static entt::DefaultRegistry editorRegistry;
		static std::vector<BaseSystem*> m_systemsList;
	};
}