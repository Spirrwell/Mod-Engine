#include "commandlinesystem.hpp"
#include "engine.hpp"
#include "log.hpp"

void CommandLineSystem::configure( Engine *engine )
{
	EngineSystem::configure( engine );

	const int argc = engine->GetArgC();
	const char * const *argv = engine->GetArgV();

	for ( int arg = 0; arg < argc; ++arg )
	{
		if ( argv[ arg ][ 0 ] == '-' ) {
			if ( argv[ arg ][ 1 ] == '-' ) {
				std::string option = argv[ arg ];

				if ( HasOption( option ) ) {
					Log::PrintlnWarn( "Duplicate option {} found!", option );
				}
				else {
					options.push_back( argv[ arg ] );
				}
			}
			else {
				const std::string argument = argv[ arg ];

				if ( HasArgument( argument ) ) {
					Log::PrintlnWarn( "Duplicate argument {} found!", argument );
					continue;
				}

				std::vector< std::string > &input = argumentMap[ argument ];

				for ( int i = arg + 1; i < argc; ++i )
				{
					// Found new argument or option, no more input
					if ( argv[ i ][ 0 ] == '-' ) {
						break;
					}

					input.push_back( argv[ i ] );
				}

				if ( input.size() == 0 ) {
					engine->Error( fmt::format( "No input for argument \"{}\"", argument ) );
				}

				argumentMap[ argument ] = input;
			}
		}
	}
}

std::vector< std::string > CommandLineSystem::GetArgumentInput( const std::string &argument ) const
{
	const auto it = argumentMap.find( argument );

	if ( it == argumentMap.cend() ) {
		Log::PrintlnWarn( "Failed to find input for argument \"{}\"", argument );
		return {};
	}

	return it->second;
}

bool CommandLineSystem::HasArgument( const std::string &argument ) const
{
	const auto it = argumentMap.find( argument );

	if ( it != argumentMap.cend() ) {
		return true;
	}

	return false;
}

bool CommandLineSystem::HasOption( const std::string &option ) const
{
	for ( const auto &str : options )
	{
		if ( str == option ) {
			return true;
		}
	}

	return false;
}