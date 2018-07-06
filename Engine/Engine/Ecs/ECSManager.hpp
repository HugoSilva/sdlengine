#pragma once

#include "Component.hpp"
#include "System.hpp"
#include <map>

namespace ecs
{
	class ECSManager
	{
	public:
		ECSManager() {}
		~ECSManager();

		EntityHandle makeEntity(ComponentBase* components, const unsigned int* componentIDs, size_t numComponents);
		void removeEntity(EntityHandle handle);

		template<class Component>
		void addComponent(EntityHandle entity, Component* component);

		template<class Component>
		void removeComponent(EntityHandle entity);

		template<class Component>
		void getComponent(EntityHandle entity);

		inline void addSystem(SystemBase& system)
		{
			systems.push_back(&system);
		}
		void updateSystems(float delta);
		void removeSystem(SystemBase& system);

	private:
		std::vector<SystemBase*> systems;
		std::map<unsigned int, std::vector<char>> components;
		std::vector<std::pair<unsigned int, std::vector<std::pair<unsigned int, unsigned int>>>* > entities;

		inline std::pair<unsigned int, std::vector<std::pair<unsigned int, unsigned int>>>* handleToRawType(EntityHandle handle)
		{
			(std::pair<unsigned int, std::vector<std::pair<unsigned int, unsigned int>>>*)handle;
		}

		inline unsigned int handleToEntityIndex(EntityHandle handle)
		{
			return handleToRawType(handle)->first;
		}

		inline std::vector<std::pair<unsigned int, unsigned int> >& handleToEntity(EntityHandle handle)
		{
			return handleToRawType(handle)->second;
		}
	};
}