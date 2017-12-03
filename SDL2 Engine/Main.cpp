#include "Engine/Application.h"

#include <iostream>

int main(int argc, char **argv)
{
	Application app;

	bool running = true;
	do {
		running = app.Run();
	}
	while (running);

	return EXIT_SUCCESS;
}