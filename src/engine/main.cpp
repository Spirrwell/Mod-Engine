#include <iostream>
#include <string_view>

#include <SDL2/SDL.h>

#include "memory.hpp"
#include "engine.hpp"


int main( int argc, char ** argv )
{
	Engine *engine = Engine::CreateEngine( argc, argv );
	engine->Run();

	Engine::DestroyEngine( engine );

	return 0;
}