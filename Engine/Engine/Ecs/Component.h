#pragma once

struct Entity; //Forward declaration

namespace ecs
{
	//Need to create a Component Manager to take care of all the component high level data
	using ComponentID = std::size_t;

	inline ComponentID GetComponentTypeID()
	{
		static ComponentID lastID = 0;
		return lastID++;
	}

	template <typename T>
	inline ComponentID GetComponentTypeID() noexcept
	{
		static ComponentID typeID = GetComponentTypeID();
		return typeID();
	}
	
	class Component
	{
	public:
		virtual ~Component() {}
		virtual void OnStart() {}
		virtual void OnEnd() {}

		Entity* m_Entity;
	};
}