#pragma once

#include "Core.h"

#include <signal/dispatcher.hpp>

class EditorManager
{
public:

	struct CreateEntity 
	{ 
		CreateEntity() 
		{
		}
	};
    struct SelectEntity { int value; };

	EditorManager() {}
	~EditorManager() {}

	void OnInit();
	static void init();
	static bool* getShowSceneWidget();
    static bool* getShowComponentWidget();
    static inline entt::Dispatcher* getEventDispatcher() { return &eventDispatcher; }
	void receive(const CreateEntity &event);
	void receive(const SelectEntity &event);

private:

	static bool m_ShowSceneWidget;
	static bool m_ShowComponentWidget;
    static ecs::EntityHandle m_SelectedEntity;
    static entt::Dispatcher eventDispatcher;
	entt::Dispatcher m_eventDispatcher;
};

