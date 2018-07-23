#pragma once

#include "ECS.hpp"
#include "System.hpp"
#include "ComponentDefinitions.hpp"
#include <vector>

namespace ecs
{
	class ECSManager
	{
	public:
		ECSManager() { }

		static void init();

		template<class... TArgs>
		static void addEntity(TArgs... arguments)
		{
			m_ecs.makeEntity((arguments)...);
		}

		static void addSystem(SystemBase& system);

		static void update(float delta);
		static void clean();

		static bool save();

	private:
		static std::vector<SystemList*> m_SystemLists;
		static ECS m_ecs;
	};
}