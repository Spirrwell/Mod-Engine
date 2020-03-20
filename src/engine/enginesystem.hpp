#ifndef ENGINESYSTEM_HPP
#define ENGINESYSTEM_HPP

#include <string>

class Engine;

class EngineSystem
{
public:
	EngineSystem() = default;
	virtual ~EngineSystem() = default;

	virtual void configure( Engine *engine );
	virtual void unconfigure( Engine *engine );

protected:
	Engine *engine = nullptr;
};

#endif // ENGINESYSTEM_HPP