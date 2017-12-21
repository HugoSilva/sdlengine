#include "Engine/Graphics/Window.h"
#include "Engine/Logger.h"

#include <iostream>

int main(int argc, char **argv)
{
	using namespace graphics;
	
	try
	{
		Window window("SDL Example!", 1280, 720);
		window.Init();

		while (window.GetRunning())
		{
			window.Run();
		}
	}
	catch (...)
	{
		Logger::error("Failed to start the application");
	}

	return EXIT_SUCCESS;
}