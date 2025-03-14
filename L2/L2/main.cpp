#include <windows.h>
#include"game.h"
int WINAPI wWinMain(HINSTANCE instance,
	HINSTANCE /*prevInstance*/,
	LPWSTR /*command_line*/,
	int show_command)
{
	game app{ instance };
	return app.run(show_command);
}