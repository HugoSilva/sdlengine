#include "ThreadManager.h"


unsigned int ThreadManager::m_AvailableThreads;
std::vector<std::thread*> ThreadManager::m_Threads;

void ThreadManager::Init()
{
	m_AvailableThreads = std::thread::hardware_concurrency();
}

void ThreadManager::Start()
{
	for (int num = 0; num < m_AvailableThreads; num++)
	{
		m_Threads.push_back(new std::thread(Physics::loop, &swapper_physics));
	}
}

void ThreadManager::Clean()
{
	for (int i = 0; i < m_AvailableThreads; i++)
	{
		m_Threads[i]->join();
		//delete m_Threads[i];
	}
}