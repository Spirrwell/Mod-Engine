#include "log.hpp"
#include <iostream>

class DefaultLogEngine : public ILogEngine
{
public:
	void WPrint( const std::wstring_view msg ) override
	{
		std::wcout << msg;
	}

	void WPrintln( const std::wstring_view msg ) override
	{
		std::wcout << msg << std::endl;
	}

	void Print( const std::string_view msg ) override
	{
		std::cout << msg;
	}

	void Println( const std::string_view msg ) override
	{
		std::cout << msg << std::endl;
	}
};

static DefaultLogEngine s_DefaultLogEngine;

ILogEngine *Log::s_pLogEngine = &s_DefaultLogEngine;
std::mutex Log::rainbowMutex;

void Log::configure( ILogEngine *logEngine )
{
	s_pLogEngine = logEngine;
}

void Log::unconfigure()
{
	s_pLogEngine = &s_DefaultLogEngine;
}