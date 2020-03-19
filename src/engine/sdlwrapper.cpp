#include "sdlwrapper.hpp"

SDLWrapper::SDLWrapper( Uint32 flags )
{
	if ( SDL_Init( flags ) ) {
		isValid = false;
	}
	else
		isValid = true;
}

SDLWrapper::~SDLWrapper()
{
	SDL_Quit();
}