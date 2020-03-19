#ifndef SDLWINDOW_HPP
#define SDLWINDOW_HPP

#include <SDL2/SDL.h>

class SDLWindow
{
public:
	SDLWindow( const char *title, int x, int y, int w, int h, Uint32 flags );
	~SDLWindow();

	SDL_Window *get() const { return window; }

private:
	SDL_Window *window = nullptr;
};

#endif // SDLWINDOW_HPP