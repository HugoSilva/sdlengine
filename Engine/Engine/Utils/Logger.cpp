#include "Logger.hpp"

#include <deque>
#include <mutex>
#include <sstream>

std::string Logger::s_WarningPrefix{"Warning: "};
std::string Logger::s_ErrorPrefix{"Error:   "};

namespace {
// Systems can log from worker threads, so the history is guarded.
std::mutex s_Mutex{};
std::deque<Logger::Entry> s_Entries{};
uint64_t s_Revision{0};
}  // namespace

void Logger::append(Level level, const std::string& message) {
  std::lock_guard<std::mutex> lock{s_Mutex};

  s_Entries.push_back(Entry{level, message});
  while (s_Entries.size() > s_MaxEntries) {
    s_Entries.pop_front();
  }

  ++s_Revision;
}

void Logger::log(std::string msg) {
  std::cout << msg << std::endl;
  append(Level::Log, msg);
}

void Logger::warning(std::string msg) {
  std::cerr << s_WarningPrefix << msg << std::endl;
  append(Level::Warning, msg);
}

void Logger::error(std::string msg) {
  std::cerr << s_ErrorPrefix << msg << std::endl;
  append(Level::Error, msg);
}

std::vector<Logger::Entry> Logger::getEntries() {
  std::lock_guard<std::mutex> lock{s_Mutex};
  return std::vector<Entry>{s_Entries.begin(), s_Entries.end()};
}

uint64_t Logger::getRevision() {
  std::lock_guard<std::mutex> lock{s_Mutex};
  return s_Revision;
}

void Logger::clear() {
  std::lock_guard<std::mutex> lock{s_Mutex};
  s_Entries.clear();
  ++s_Revision;
}
