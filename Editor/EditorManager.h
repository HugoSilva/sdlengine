#pragma once

#include "Core.h"
#define INVALID_ENTITY_ID entt::null

enum class eComponents { Audio, Rigidbody, Sprite };

class EditorManager
{
public:

	struct CreateEntity { };
    struct SelectEntity { uint32_t value; };
	struct AddComponent { eComponents component; };

	EditorManager() {}
	~EditorManager() {}

	static void init();
	static bool* getShowSceneWidget();
    static bool* getShowComponentWidget();

    template<typename Component>
    static bool showComponentWidget()
    {
        if (m_SelectedEntity != INVALID_ENTITY_ID)
        {
            return rse::ECSManager::hasComponent<Component>(m_SelectedEntity);
        }
		return false;
    }

	template<typename Component>
	static Component& getComponent()
	{
		return rse::ECSManager::getComponent<Component>(m_SelectedEntity);
	}

	void receive(const CreateEntity &event);
	void receive(const SelectEntity &event);
	void receive(const AddComponent &event);

private:

	static bool m_ShowSceneWidget;
	static bool m_ShowComponentWidget;
    static uint32_t m_SelectedEntity;
};

