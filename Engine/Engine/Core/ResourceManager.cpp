#include "ResourceManager.hpp"

#include "Resource.hpp"

std::vector<Resource*> ResourceManager::s_Resources;

void ResourceManager::init()
{
}

void ResourceManager::add(Resource* resource)
{
	for (std::vector<Resource*>::iterator it = s_Resources.begin(); it != s_Resources.end(); ++it)
	{
		Resource* res = *it;
		if (res->getFileName().compare(resource->getFileName()) == 0)
			return;
	}

	s_Resources.push_back(resource);
}

Resource* ResourceManager::get(const std::string& name)
{
	for (std::vector<Resource*>::iterator it = s_Resources.begin(); it != s_Resources.end(); ++it)
	{
		Resource* res = *it;
		if (res->getFileName().compare(name) == 0)
			return res;
	}
	return nullptr;
}

void ResourceManager::clean()
{
	s_Resources.clear();
}