#include "Logger.hpp"

#include <sstream>

std::string Logger::s_WarningPrefix{ "Warning: " };
std::string Logger::s_ErrorPrefix{ "Error:   " };

void Logger::log(std::string msg)
{
	std::cout << msg << std::endl;
}

void Logger::warning(std::string msg)
{
	std::cerr << s_WarningPrefix << msg << std::endl;
}

void Logger::error(std::string msg)
{
	std::cerr << s_ErrorPrefix << msg << std::endl;
}