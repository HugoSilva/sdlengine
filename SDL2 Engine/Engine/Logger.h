#ifndef LOGGER
#define LOGGER

#include <iostream>
#include <fstream>
#include <string>

class Logger
{
public:
	static void log(std::string msg);
	static void warning(std::string msg);
	static void error(std::string msg);

private:
	static std::string errorPrefix;
	static std::string warningPrefix;
};

#endif //LOGGER