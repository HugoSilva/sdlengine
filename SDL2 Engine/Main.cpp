#include "Engine/Application.h"
#include "Engine/Logger.h"

#include <iostream>

int main(int argc, char **argv)
{
	try
	{
		Application app;

		bool running = true;
		do {
			running = app.Run();
		} while (running);
	}
	catch (...)
	{
		Logger::error("Failed to start the application");
	}

	return EXIT_SUCCESS;
}