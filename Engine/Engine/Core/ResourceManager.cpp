#include "ResourceManager.h"

std::vector<Resource*> ResourceManager::m_Resources;

void ResourceManager::init()
{
}

void ResourceManager::add(Resource* resource)
{
	for (std::vector<Resource*>::iterator it = m_Resources.begin(); it != m_Resources.end(); ++it)
	{
		Resource* res = *it;
		if (res->getFileName().compare(resource->getFileName()) == 0)
			return;
	}

	m_Resources.push_back(resource);
}

Resource* ResourceManager::get(const std::string& name)
{
	for (std::vector<Resource*>::iterator it = m_Resources.begin(); it != m_Resources.end(); ++it)
	{
		Resource* res = *it;
		if (res->getFileName().compare(name) == 0)
			return res;
	}
	return nullptr;
}

void ResourceManager::clean()
{
	m_Resources.clear();
}