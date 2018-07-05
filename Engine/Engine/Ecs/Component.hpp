#pragma once

namespace ecs
{
	typedef void* EntityHandle;

	struct ComponentBase
	{
		EntityHandle entityID;
		static unsigned int nextID();
	};

	template<typename T>
	struct Component : public ComponentBase
	{
		static const ComponentCreateFunction CREATE_FUNCTION;
		static const ComponentFreeFunction FREE_FUNCTION;
		static const unsigned int ID;
		static const unsigned int SIZE;
	};

	template<typename T>
	const unsigned int ComponentBase<T>::ID(ComponentBase::nextID());

	template<typename T>
	const unsigned int ComponentBase<T>::SIZE(sizeof(T));

	struct PositionComponent : public Component<PositionComponent>
	{
		float x;
		float y;
		float z;
	};
}