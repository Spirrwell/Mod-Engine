#include "enginesystem.hpp"
#include "engine.hpp"

void EngineSystem::configure( Engine *engine )
{
	this->engine = engine;
}

void EngineSystem::unconfigure( Engine *engine )
{
	this->engine = nullptr;
}