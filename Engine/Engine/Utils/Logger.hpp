#pragma once

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
	static std::string s_ErrorPrefix;
	static std::string s_WarningPrefix;
};