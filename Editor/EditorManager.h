#pragma once

#include "Core.h"

#include <signal/dispatcher.hpp>

class EditorManager
{
public:

    struct CreateEntity { };
    struct SelectEntity { int value; };

	EditorManager() {}
	~EditorManager() {}

	static void init();
	static bool* getShowSceneWidget();
    static bool* getShowComponentWidget();
    static inline entt::Dispatcher* getEventDispatcher() { return &eventDispatcher; }

private:
	void createNewEntity(const CreateEntity &event);
	void selectEntity(const SelectEntity &event);

	static bool m_ShowSceneWidget;
	static bool m_ShowComponentWidget;
    static ecs::EntityHandle m_SelectedEntity;
    static entt::Dispatcher eventDispatcher;
};

