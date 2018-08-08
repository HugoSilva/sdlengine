#pragma once

#include "Core.h"

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

	static void init();
	static bool* getShowSceneWidget();
    static bool* getShowComponentWidget();

	void receive(const CreateEntity &event);
	void receive(const SelectEntity &event);

private:

	static bool m_ShowSceneWidget;
	static bool m_ShowComponentWidget;
    static ecs::EntityHandle m_SelectedEntity;
};

