#ifndef SDLWRAPPER_HPP
#define SDLWRAPPER_HPP

#include <SDL2/SDL.h>
#include <vector>

class SDLWrapper
{
public:
	SDLWrapper( Uint32 flags );
	~SDLWrapper();

	bool is_valid() const { return isValid; }

private:
	bool isValid = false;
};

#endif // SDLWRAPPER_HPP