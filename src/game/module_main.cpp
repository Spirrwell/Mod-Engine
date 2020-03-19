#include "platform.hpp"
#include "engine/iengine.hpp"
#include "log.hpp"

#include "game.hpp"

Game *game = nullptr;

extern "C" EXPORT void module_configure( IEngine *engine )
{
	Log::configure( engine );
	game = new Game;
	engine->SetGame( game );
}

extern "C" EXPORT void module_unconfigure( IEngine *engine )
{
	// Note: Unloading the game module implicitly sets the engine's game to nullptr
	delete game;
	game = nullptr;
	Log::unconfigure();
}