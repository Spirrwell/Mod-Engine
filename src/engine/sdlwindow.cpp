#include "sdlwindow.hpp"

SDLWindow::SDLWindow( const char *title, int x, int y, int w, int h, Uint32 flags )
{
	window = SDL_CreateWindow( title, x, y, w, h, flags );
}

SDLWindow::~SDLWindow()
{
	SDL_DestroyWindow( window );
}