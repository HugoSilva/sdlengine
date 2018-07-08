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
		inline void addComponent(EntityHandle entity, Component* component)
		{
			addComponentInternal(entity, handleToEntity(entity), Component::ID, component);
		}

		template<class Component>
		inline bool removeComponent(EntityHandle entity)
		{
			return removeComponentInternal(entity, Component::ID);
		}

		template<class Component>
		inlineComponent* getComponent(EntityHandle entity)
		{
			getComponentInternal(handleToEntity(entity), components[Component::ID], Component::ID);
		}

		inline void addSystem(SystemBase& system)
		{
			systems.push_back(&system);
		}
		void updateSystems(float delta);
		bool removeSystem(SystemBase& system);

	private:
		std::vector<SystemBase*> systems;
		std::map<unsigned int, std::vector<char>> components;
		std::vector<std::pair<unsigned int, std::vector<std::pair<unsigned int, unsigned int>>>* > entities;

		inline std::pair<unsigned int, std::vector<std::pair<unsigned int, unsigned int>>>* handleToRawType(EntityHandle handle)
		{
			return (std::pair<unsigned int, std::vector<std::pair<unsigned int, unsigned int>>>*)handle;
		}

		inline unsigned int handleToEntityIndex(EntityHandle handle)
		{
			return handleToRawType(handle)->first;
		}

		inline std::vector<std::pair<unsigned int, unsigned int> >& handleToEntity(EntityHandle handle)
		{
			return handleToRawType(handle)->second;
		}

		void deleteComponent(unsigned int componentID, unsigned int index);
		bool removeComponentInternal(EntityHandle handle, unsigned int componentID);
		void addComponentInternal(EntityHandle handle, std::vector<std::pair<unsigned int, unsigned int> >& entity, unsigned int componentID, ComponentBase* component);
		ComponentBase* getComponentInternal(std::vector<std::pair<unsigned int, unsigned int> >& entityComponents, unsigned int componentID);

		ComponentBase* getComponentInternal(std::vector<std::pair<unsigned int, unsigned int> >& entityComponents, std::vector<char>& array, unsigned int componentID);
		
		void updateSystemWithMultipleComponents(unsigned int index, float delta, const std::vector<unsigned int>& componentTypes, 
			std::vector<ComponentBase*>& componentParam, std::vector<std::vector<char>*>& componentArrays);
		
		unsigned int findLeastCommonComponent(const std::vector<unsigned int>& componentTypes);
	};
}