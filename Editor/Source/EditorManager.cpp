#include "EditorManager.h"

uint32_t EditorManager::m_SelectedEntity{ INVALID_ENTITY_ID };
rse::Framebuffer* EditorManager::m_SceneTarget{ nullptr };
EditorSettings EditorManager::m_Settings;
bool EditorManager::m_FrameRequested{ false };

void EditorManager::init()
{
	// Before anything can read a setting or draw a panel with one. Called from
	// the setup view's constructor, so it happens once.
	m_Settings.load();

	EventManager::getEventDispatcher()->sink<CreateEntity>().connect<&EditorManager::handleCreateEntityEvent>();
	EventManager::getEventDispatcher()->sink<SelectEntity>().connect<&EditorManager::handleSelectEntityEvent>();
	EventManager::getEventDispatcher()->sink<DestroyEntity>().connect<&EditorManager::handleDestroyEntityEvent>();
	EventManager::getEventDispatcher()->sink<AddComponent>().connect<&EditorManager::handleAddComponentEvent>();
	EventManager::getEventDispatcher()->sink<RemoveComponent>().connect<&EditorManager::handleRemoveComponentEvent>();
}

void EditorManager::handleCreateEntityEvent(const CreateEntity& event)
{
	m_SelectedEntity = rse::ECSManager::createEntity();
	rse::ECSManager::addComponent<BasicComponent>(m_SelectedEntity);
	rse::ECSManager::addComponent<TransformComponent>(m_SelectedEntity);
	rse::ECSManager::addComponent<SpriteComponent>(m_SelectedEntity, new rse::Sprite(glm::vec3(0, 0, 0), glm::vec2(40, 40), rse::TextureManager::get("test00")));
}

void EditorManager::handleSelectEntityEvent(const SelectEntity& event)
{
	setSelection(event.value);
}

std::span<const uint32_t> EditorManager::getSelectedEntities()
{
	// One element, materialised on demand: the storage is still the single
	// active entity, so there is no second copy of the selection to keep in
	// step with the first. The span is valid until the selection changes.
	static uint32_t active[1];

	if (m_SelectedEntity == INVALID_ENTITY_ID)
	{
		return {};
	}

	active[0] = m_SelectedEntity;
	return std::span<const uint32_t>(active, 1);
}

void EditorManager::handleDestroyEntityEvent(const DestroyEntity& event)
{
	if (event.value == INVALID_ENTITY_ID)
	{
		return;
	}

	// Drop the selection first: the inspector must not keep reading a dead id.
	if (m_SelectedEntity == event.value)
	{
		m_SelectedEntity = INVALID_ENTITY_ID;
	}

	rse::ECSManager::destroyEntity(event.value);
}

void EditorManager::handleAddComponentEvent(const AddComponent& event)
{
	switch (event.component)
	{
		case eComponents::Audio:
			if (rse::ECSManager::hasComponent<AudioComponent>(m_SelectedEntity))
			{
				Logger::error("Already existing audio component");
			}
			else
			{
				rse::ECSManager::addComponent<AudioComponent>(m_SelectedEntity, rse::SoundManager::getSound("eff"));
			}
			break;
		case eComponents::Rigidbody:
			if (rse::ECSManager::hasComponent<RigidBodyComponent>(m_SelectedEntity))
			{
				Logger::error("Already existing rigidbody component");
			}
			else
			{
				glm::vec2 position{ 0.f, 0.f };
				glm::vec2 size{ 0.f, 0.f };
				if (rse::ECSManager::hasComponent<TransformComponent>(m_SelectedEntity))
				{
					position = rse::ECSManager::getComponent<TransformComponent>(m_SelectedEntity).position;
				}
				if (rse::ECSManager::hasComponent<SpriteComponent>(m_SelectedEntity))
				{
					size = rse::ECSManager::getComponent<SpriteComponent>(m_SelectedEntity).m_Sprite->getSize();
				}
				rse::ECSManager::addComponent<RigidBodyComponent>(m_SelectedEntity, new Rigidbody(position, size));
			}
			break;
		case eComponents::Sprite:
			if (rse::ECSManager::hasComponent<SpriteComponent>(m_SelectedEntity))
			{
				Logger::error("Already existing sprite component");
			}
			else
			{
				rse::ECSManager::addComponent<SpriteComponent>(m_SelectedEntity);
			}
			break;
		default:
			break;
	}
}

void EditorManager::handleRemoveComponentEvent(const RemoveComponent& event)
{
	if (m_SelectedEntity == INVALID_ENTITY_ID)
	{
		return;
	}

	// Only the optional components can go: the hierarchy lists entities by their
	// BasicComponent and the renderer needs a transform, so those two stay.
	switch (event.component)
	{
		case eComponents::Audio:
			if (!rse::ECSManager::hasComponent<AudioComponent>(m_SelectedEntity))
			{
				Logger::error("Missing audio component");
				break;
			}
			// Components hold their payload in a shared_ptr, so removing one
			// releases it properly.
			rse::ECSManager::removeComponent<AudioComponent>(m_SelectedEntity);
			break;
		case eComponents::Rigidbody:
			if (!rse::ECSManager::hasComponent<RigidBodyComponent>(m_SelectedEntity))
			{
				Logger::error("Missing rigidbody component");
				break;
			}
			rse::ECSManager::removeComponent<RigidBodyComponent>(m_SelectedEntity);
			break;
		case eComponents::Sprite:
			if (!rse::ECSManager::hasComponent<SpriteComponent>(m_SelectedEntity))
			{
				Logger::error("Missing sprite component");
				break;
			}
			rse::ECSManager::removeComponent<SpriteComponent>(m_SelectedEntity);
			break;
		default:
			break;
	}
}