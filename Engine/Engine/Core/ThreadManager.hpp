#pragma once

#include <mutex>
#include <thread>
#include <vector>

class ThreadManager
{
public:
	static void init();
	static void start();
	static void clean();

private:
	ThreadManager() { }

	static unsigned int s_AvailableThreads;
	static std::vector<std::thread*> s_Threads;
	static std::vector<std::mutex*> s_Mutexes;
};