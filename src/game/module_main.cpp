#include "platform.hpp"
#include "engine/iengine.hpp"
#include "log.hpp"

#include "game.hpp"

extern "C" EXPORT void module_configure( IEngine *engine )
{
	Log::configure( engine );
	engine->SetGame( new Game );
}

extern "C" EXPORT void module_unconfigure( IEngine *engine )
{
	// Note: The engine deletes the game pointer
	Log::unconfigure();
}