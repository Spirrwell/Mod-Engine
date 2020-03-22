#include <iostream>
#include <string_view>

#include <SDL2/SDL.h>

#include "memory.hpp"
#include "engine.hpp"

#ifdef _WIN32
// This is a bit silly, but I don't want to include Windows.h because I'm stubborn
#include <wtypes.h>
#include <minwindef.h>
#include <WinBase.h>
#include <processenv.h>
#include <consoleapi.h>
#endif

int main( int argc, char ** argv )
{
#ifdef _WIN32
	auto consoleHandle = GetStdHandle( STD_OUTPUT_HANDLE );
	DWORD flags = {};
	GetConsoleMode( consoleHandle, &flags );
	SetConsoleMode( consoleHandle, flags | ENABLE_VIRTUAL_TERMINAL_PROCESSING );
#endif
	Engine *engine = Engine::CreateEngine( argc, argv );
	engine->Run();

	Engine::DestroyEngine( engine );

	return 0;
}