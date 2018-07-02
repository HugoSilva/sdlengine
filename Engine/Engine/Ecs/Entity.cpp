#include "Entity.h"

namespace ecs
{
	Entity::Entity()
	{
	}

	template <typename T, typename... TArgs>
	void Entity::AddComponent(TArgs&&... arguments)
	{
		T* component(new T(std::forward<TArgs>(arguments)));
		component->entity = this;
		m_Components.push_back(component);

		m_ComponentArray[GetComponentTypeID<T>()] = component;
		m_ComponentBitset[GetComponentTypeID<T>()] = true;

		component->
	}

	template <typename T>
	T& Entity::GetComponent() const
	{
		auto ptr(m_ComponentArray[GetComponentTypeID<T>()]);
		return *static_cast<T*>(ptr);
	}

	template <typename T>
	void Entity::RemoveComponent()
	{

	}

	void Entity::RemoveAllComponents()
	{

	}

	template <typename T>
	bool Entity::HasComponent() const
	{
		return m_ComponentBitset[GetComponentTypeID<T>()];
	}

	void Entity::writeXML(rapidxml::xml_document<>* doc, rapidxml::xml_node<>* parentNode)
	{
		rapidxml::xml_node<> *component = doc->allocate_node(rapidxml::node_element, "Component");
		parentNode->append_node(component);

		char *attr_id = doc->allocate_string(std::to_string(m_id).c_str());
		component->append_attribute(doc->allocate_attribute("id", attr_id));

		rapidxml::xml_node<> *position = doc->allocate_node(rapidxml::node_element, "Position");
		component->append_node(position);

		char *attr_position_x = doc->allocate_string(std::to_string(m_position.x).c_str());
		position->append_attribute(doc->allocate_attribute("x", attr_position_x));
	}

	void Entity::readXML(rapidxml::xml_document<>* doc, rapidxml::xml_node<>* node)
	{
		m_id = atoi(node->first_attribute("id")->value());
		rapidxml::xml_node<>* positionNode = node->first_node("Position");
		m_position.x = atof(positionNode->first_attribute("x")->value());
	}
}