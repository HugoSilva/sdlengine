#include "Engine/Graphics/Window.h"
#include "Engine/Logger.h"
#include "Examples/ModelTest.h"
#include "Examples/SpriteTest.h"

#include <iostream>

int main(int argc, char **argv)
{
	using namespace graphics;
	using namespace examples;
	
	try
	{
		Window window("SDL Example!", 1280, 720);
		window.Init();

		//ModelTest* scene = new ModelTest();
		SpriteTest* scene = new SpriteTest(window.GetWindow());
		window.LoadScene(scene);

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