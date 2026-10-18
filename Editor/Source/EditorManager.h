#pragma once

#include "Core.h"
#define INVALID_ENTITY_ID entt::null

enum class eComponents { Audio, Rigidbody, Sprite };

class EditorManager
{
public:

	struct CreateEntity { };
    struct SelectEntity { uint32_t value; };
	struct DestroyEntity { uint32_t value; };
	struct AddComponent { eComponents component; };
	struct RemoveComponent { eComponents component; };

	EditorManager() {}
	~EditorManager() {}

	static void init();

	// Selection is shared state: the hierarchy panel drives it, the inspector
	// panel reads it.
	static void setSelectedEntity(uint32_t entity) { m_SelectedEntity = entity; }
	static uint32_t getSelectedEntity() { return m_SelectedEntity; }

	// The scene is rendered into the viewport panel's framebuffer rather than
	// the window. The viewport claims this while it draws; Core::run() picks it
	// up for the scene pass, and nullptr means "draw straight to the window".
	static void setSceneTarget(rse::Framebuffer* target) { m_SceneTarget = target; }
	static rse::Framebuffer* getSceneTarget() { return m_SceneTarget; }

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

	static void handleCreateEntityEvent(const CreateEntity& event);
	static void handleSelectEntityEvent(const SelectEntity& event);
	static void handleDestroyEntityEvent(const DestroyEntity& event);
	static void handleAddComponentEvent(const AddComponent& event);
	static void handleRemoveComponentEvent(const RemoveComponent& event);

private:

    static uint32_t m_SelectedEntity;
	static rse::Framebuffer* m_SceneTarget;
};
