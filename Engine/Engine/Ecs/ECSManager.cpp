#include "ECSManager.hpp"

#include <algorithm>

namespace ecs
{
	ECSManager::~ECSManager()
	{
		for (std::map<unsigned int, std::vector<char>>::iterator it = components.begin(); it != components.end(); ++it) {
			size_t typeSize = ComponentBase::getTypeSize(it->first);
			ComponentFreeFunction freefn = ComponentBase::getTypeFreeFunction(it->first);
			for (unsigned int i = 0; i < it->second.size(); i += typeSize) {
				freefn((ComponentBase*)&it->second[i]);
			}
		}

		for (unsigned int i = 0; i < entities.size(); i++) {
			delete entities[i];
		}
	}
	EntityHandle ECSManager::makeEntity(ComponentBase** entityComponents, const unsigned int* componentIDs, size_t numComponents)
	{
		std::pair<unsigned int, std::vector<std::pair<unsigned int, unsigned int> > >* newEntity = new std::pair<unsigned int, std::vector<std::pair<unsigned int, unsigned int> > >();
		EntityHandle handle = (EntityHandle)newEntity;
		for (unsigned int i = 0; i < numComponents; i++) {
			if (!ComponentBase::isTypeValid(componentIDs[i])) {
				delete newEntity;
				return NULL_ENTITY_HANDLE;
			}

			addComponentInternal(handle, newEntity->second, componentIDs[i], entityComponents[i]);
		}

		newEntity->first = entities.size();
		entities.push_back(newEntity);
		return handle;
	}

	void ECSManager::removeEntity(EntityHandle handle)
	{
		std::vector<std::pair<unsigned int, unsigned int> >& entity = handleToEntity(handle);
		for (unsigned int i = 0; i < entity.size(); i++) {
			deleteComponent(entity[i].first, entity[i].second);
		}

		unsigned int destIndex = handleToEntityIndex(handle);
		unsigned int srcIndex = entities.size() - 1;
		delete entities[destIndex];
		entities[destIndex] = entities[srcIndex];
		entities.pop_back();
	}

	void ECSManager::addComponentInternal(EntityHandle handle, std::vector<std::pair<unsigned int, unsigned int> >& entity, unsigned int componentID, ComponentBase* component)
	{
		ComponentCreateFunction createfn = ComponentBase::getTypeCreateFunction(componentID);
		std::pair<unsigned int, unsigned int> newPair;
		newPair.first = componentID;
		newPair.second = createfn(components[componentID], handle, component);
		entity.push_back(newPair);
	}

	void ECSManager::deleteComponent(unsigned int componentID, unsigned int index)
	{
		std::vector<char>& array = components[componentID];
		ComponentFreeFunction freefn = ComponentBase::getTypeFreeFunction(componentID);
		size_t typeSize = ComponentBase::getTypeSize(componentID);
		unsigned int srcIndex = array.size() - typeSize;

		ComponentBase* destComponent = (ComponentBase*)&array[index];
		ComponentBase* srcComponent = (ComponentBase*)&array[srcIndex];
		freefn(destComponent);

		if (index == srcIndex) {
			array.resize(srcIndex);
			return;
		}

		::memcpy(destComponent, srcComponent, typeSize);

		std::vector<std::pair<unsigned int, unsigned int> >& srcComponents = handleToEntity(srcComponent->entity);
		for (unsigned int i = 0; i < srcComponents.size(); i++) {
			if (componentID == srcComponents[i].first && srcIndex == srcComponents[i].second) {
				srcComponents[i].second = index;
				break;
			}
		}

		array.resize(srcIndex);
	}

	bool ECSManager::removeComponentInternal(EntityHandle handle, unsigned int componentID)
	{
		std::vector<std::pair<unsigned int, unsigned int> >& entityComponents = handleToEntity(handle);
		for (unsigned int i = 0; i < entityComponents.size(); i++) {
			if (componentID == entityComponents[i].first) {
				deleteComponent(entityComponents[i].first, entityComponents[i].second);
				unsigned int srcIndex = entityComponents.size() - 1;
				unsigned int destIndex = i;
				entityComponents[destIndex] = entityComponents[srcIndex];
				entityComponents.pop_back();
				return true;
			}
		}
		return false;
	}

	ComponentBase* ECSManager::getComponentInternal(std::vector<std::pair<unsigned int, unsigned int>>& entityComponents, std::vector<char>& array, unsigned int componentID);
	{
		for (unsigned int i = 0; i < entityComponents.size(); i++) {
			if (componentID == entityComponents[i].first) {
				return (ComponentBase*)&components[componentID][entityComponents[i].second];
			}
		}
		return nullptr;
	}

	void ECSManager::updateSystems(SystemList& systems, float delta)
	{
		std::vector<ComponentBase*> componentParam;
		std::vector<std::vector<char>*> componentArrays;
		for (unsigned int i = 0; i < systems.size(); i++) {
			const std::vector<unsigned int>& componentTypes = systems[i]->getComponentTypes();
			if (componentTypes.size() == 1) {
				size_t typeSize = ComponentBase::getTypeSize(componentTypes[0]);
				std::vector<char>& array = components[componentTypes[0]];
				for (unsigned int j = 0; j < array.size(); j += typeSize) {
					ComponentBase* component = (ComponentBase*)&array[j];
					systems[i]->updateComponents(delta, &component);
				}
			}
			else {
				updateSystemWithMultipleComponents(i, systems, delta, componentTypes, componentParam, componentArrays);
			}
		}
	}

	unsigned int ECSManager::findLeastCommonComponent(const std::vector<unsigned int>& componentTypes, const std::vector<unsigned int>& componentFlags)
	{
		unsigned int minSize = components[componentTypes[0]].size()
			/ ComponentBase::getTypeSize(componentTypes[0]);
		unsigned int minIndex = 0;
		for (unsigned int i = 1; i < componentTypes.size(); i++) {
			size_t typeSize = ComponentBase::getTypeSize(componentTypes[i]);
			unsigned int size = components[componentTypes[i]].size() / typeSize;
			if (size < minSize) {
				minSize = size;
				minIndex = i;
			}
		}

		return minIndex;
	}

	void ECSManager::updateSystemWithMultipleComponents(unsigned int index, SystemList& systems, float delta,
		const std::vector<unsigned int>& componentTypes, std::vector<ComponentBase*>& componentParam,
		std::vector<std::vector<char>*>& componentArrays)
	{
		const std::vector<unsigned int>& componentFlags = systems[index]->getComponentFlags();

		componentParam.resize(std::max(componentParam.size(), componentTypes.size()));
		componentArrays.resize(std::max(componentArrays.size(), componentTypes.size()));
		for (unsigned int i = 0; i < componentTypes.size(); i++)
		{
			componentArrays[i] = &components[componentTypes[i]];
		}
		unsigned int minSizeIndex = findLeastCommonComponent(componentTypes, componentFlags);

		size_t typeSize = ComponentBase::getTypeSize(componentTypes[minSizeIndex]);
		std::vector<char>& array = *componentArrays[minSizeIndex];
		for (unsigned int i = 0; i < array.size(); i += typeSize)
		{
			componentParam[minSizeIndex] = (ComponentBase*)&array[i];
			std::vector<std::pair<unsigned int, unsigned int> >& entityComponents = handleToEntity(componentParam[minSizeIndex]->entity);

			bool isValid = true;
			for (unsigned int j = 0; j < componentTypes.size(); j++)
			{
				if (j == minSizeIndex)
				{
					continue;
				}

				componentParam[j] = getComponentInternal(entityComponents, *componentArrays[j], componentTypes[j]);
				if (componentParam[j] == nullptr && (componentFlags[j] & SystemBase::FLAG_OPTIONAL) == 0)
				{
					isValid = false;
					break;
				}
			}

			if (isValid)
			{
				systems[index]->updateComponents(delta, &componentParam[0]);
			}
		}
	}
}