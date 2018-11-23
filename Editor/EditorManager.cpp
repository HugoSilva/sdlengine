#include "EditorManager.h"

bool EditorManager::m_ShowSceneWidget{ true };
bool EditorManager::m_ShowComponentWidget{ true };
uint32_t EditorManager::m_SelectedEntity{ INVALID_ENTITY_ID };

void EditorManager::init()
{
	EventManager::getEventDispatcher()->sink<CreateEntity>().connect(&EditorManager::EditorManager());
	EventManager::getEventDispatcher()->sink<SelectEntity>().connect(&EditorManager::EditorManager());
	EventManager::getEventDispatcher()->sink<AddComponent>().connect(&EditorManager::EditorManager());
}

bool* EditorManager::getShowSceneWidget()
{
	return &m_ShowSceneWidget;
}

bool* EditorManager::getShowComponentWidget()
{
	return &m_ShowComponentWidget;
}

void EditorManager::receive(const CreateEntity &event)
{
	m_SelectedEntity = rse::ECSManager::createEntity();
	rse::ECSManager::addComponent<BasicComponent>(m_SelectedEntity);
	rse::ECSManager::addComponent<TransformComponent>(m_SelectedEntity);
	rse::ECSManager::addComponent<SpriteComponent>(m_SelectedEntity, new rse::Sprite(glm::vec3(0, 0, 0), glm::vec2(40, 40), rse::TextureManager::get("test00")));
}

void EditorManager::receive(const SelectEntity &event)
{
	m_SelectedEntity = event.value;
}

void EditorManager::receive(const AddComponent &event)
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