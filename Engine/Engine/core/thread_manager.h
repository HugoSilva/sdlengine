#pragma once

#include <mutex>
#include <thread>
#include <vector>

class ThreadManager {
 public:
  static void Init();
  static void Start();
  static void Clean();

 private:
  ThreadManager() {}

  static unsigned int s_available_threads;
  static std::vector<std::thread*> s_threads;
  static std::vector<std::mutex*> s_mutexes;
};