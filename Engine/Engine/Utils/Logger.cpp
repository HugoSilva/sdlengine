#include "Logger.h"

#include <sstream>

std::string Logger::warningPrefix{ "Warning: " };
std::string Logger::errorPrefix{ "Error:   " };

void Logger::log(std::string msg)
{
	std::cout << msg << std::endl;
}

void Logger::warning(std::string msg)
{
	std::cerr << warningPrefix << msg << std::endl;
}

void Logger::error(std::string msg)
{
	std::cerr << errorPrefix << msg << std::endl;
}