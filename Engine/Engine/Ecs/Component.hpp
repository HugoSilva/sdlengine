#pragma once

#include <vector>
#include <tuple>

namespace ecs
{
	struct ComponentBase;
	typedef void* EntityHandle;
	typedef unsigned int (*ComponentCreateFunction)(std::vector<char>& memory, EntityHandle entity, ComponentBase* comp);
	typedef void (*ComponentFreeFunction)(ComponentBase* comp);
	#define NULL_ENTITY_HANDLE nullptr

	struct ComponentBase
	{
	public:
		EntityHandle entity = NULL_ENTITY_HANDLE;
		static unsigned int registerComponentType(ComponentCreateFunction createfn, ComponentFreeFunction freefn, unsigned int size);

		inline static ComponentCreateFunction getTypeCreateFunction(unsigned int id)
		{
			return std::get<0>(componentTypes[id]);
		}

		inline static ComponentFreeFunction getTypeFreeFunction(unsigned int id)
		{
			return std::get<1>(componentTypes[id]);
		}

		inline static unsigned int getTypeSize(unsigned int id)
		{
			return std::get<2>(componentTypes[id]);
		}

		inline static bool isTypeValid(unsigned int id)
		{
			return id < componentTypes.size();
		}
	private:
		static std::vector<std::tuple<ComponentCreateFunction, ComponentFreeFunction, unsigned int>> componentTypes;
	};

	template<typename T>
	struct Component : public ComponentBase
	{
		static const ComponentCreateFunction CREATE_FUNCTION;
		static const ComponentFreeFunction FREE_FUNCTION;
		static const unsigned int ID;
		static const unsigned int SIZE;
	};

	template<typename Component>
	unsigned int ECSComponentCreate(std::vector<char>& memory, EntityHandle entity, ComponentBase* comp)
	{
		unsigned int index = memory.size();
		memory.resize(index + Component::SIZE);
		Component* component = new(&memory[index])Component(*(Component*)comp);
		component->entity = entity;
		return index;
	}

	template<typename Component>
	void ECSComponentFree(ComponentBase* comp)
	{
		Component* component = (Component*)comp;
		component->~Component();
	}

	template<typename T>
	const unsigned int ComponentBase<T>::ID(ComponentBase::registerComponentType(ECSComponentCreate<T>, ECSComponentFree<T>, sizeof(T)));

	template<typename T>
	const unsigned int ComponentBase<T>::SIZE(sizeof(T));

	template<typename T>
	const ComponentCreateFunction ECSComponent<T>::CREATE_FUNCTION(ECSComponentCreate<T>);

	template<typename T>
	const ComponentFreeFunction ECSComponent<T>::FREE_FUNCTION(ECSComponentFree<T>);

	struct PositionComponent : public Component<PositionComponent>
	{
		float x;
		float y;
		float z;
	};
}