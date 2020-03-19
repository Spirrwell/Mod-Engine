#ifndef ENGINESYSTEM_HPP
#define ENGINESYSTEM_HPP

#include <string>

class Engine;

class EngineSystem
{
public:
	EngineSystem() = default;
	virtual ~EngineSystem() = default;

	virtual void configure( Engine *engine ) { this->engine = engine; }
	virtual void unconfigure( Engine *engine ) { this->engine = nullptr; }

protected:
	Engine *engine = nullptr;
};

#endif // ENGINESYSTEM_HPP