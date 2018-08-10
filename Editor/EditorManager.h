#pragma once

#include "Core.h"
#define INVALID_ENTITY_ID -1

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

    template<typename Component>
    static bool showComponentWidget()
    {
        if (m_SelectedEntity > INVALID_ENTITY_ID)
        {
            return ecs::ECSManager::hasComponent<Component>(m_SelectedEntity);
        }
    }

	void receive(const CreateEntity &event);
	void receive(const SelectEntity &event);

private:

	static bool m_ShowSceneWidget;
	static bool m_ShowComponentWidget;
    static uint32_t m_SelectedEntity;
};

