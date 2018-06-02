#pragma once

#include <mutex>
#include <thread>
#include <vector>

class ThreadManager
{
public:
	static void Init();
	static void Start();
	static void Clean();

private:
	ThreadManager() { }

	static unsigned int m_AvailableThreads;
	static std::vector<std::thread*> m_Threads;
	static std::vector<std::mutex*> m_Mutexes;
};