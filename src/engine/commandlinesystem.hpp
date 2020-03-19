#ifndef COMMANDLINESYSTEM_HPP
#define COMMANDLINESYSTEM_HPP

#include <vector>
#include <string>
#include <unordered_map>

#include "enginesystem.hpp"
#include "memory.hpp"

class CommandLineSystem : public EngineSystem
{
public:
	void configure( Engine *engine ) override;

	std::vector< std::string > GetArgumentInput( const std::string &argument ) const;

	bool HasArgument( const std::string &argument ) const;
	bool HasOption( const std::string &option ) const;

private:
	std::unordered_map< std::string, std::vector< std::string > > argumentMap;
	std::vector< std::string > options;
};

#endif // COMMANDLINESYSTEM_HPP