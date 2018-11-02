#include "ThreadManager.h"
#include "../Input/InputManager.hpp"
#include "../Physics/PhysicsManager.h"

unsigned int ThreadManager::m_AvailableThreads;
std::vector<std::thread*> ThreadManager::m_Threads;
std::vector<std::mutex*> ThreadManager::m_Mutexes;

void ThreadManager::Init()
{
	m_AvailableThreads = std::thread::hardware_concurrency();
}

void ThreadManager::Start()
{
	m_Threads.push_back(new std::thread(PhysicsManager::UpdateObjects));

	for (unsigned int num = 0; num < m_AvailableThreads; num++)
	{
		//m_Threads.push_back(&threadIO);
	}
}

void ThreadManager::Clean()
{
	for (unsigned int i = 0; i < m_AvailableThreads; i++)
	{
		m_Threads[i]->join();
		//delete m_Threads[i];
	}
}