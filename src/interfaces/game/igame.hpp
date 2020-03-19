#ifndef IGAME_HPP
#define IGAME_HPP

#include "engine/iengine.hpp"

class IGame
{
public:
	virtual ~IGame() = default;

	virtual void configure( IEngine *engine ) = 0;
	virtual void unconfigure( IEngine *engine ) = 0;

	virtual void tick( float dt ) = 0;
};

#endif // IGAME_HPP