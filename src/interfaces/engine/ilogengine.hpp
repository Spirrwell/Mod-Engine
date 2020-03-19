#ifndef ILOGENGINE_HPP
#define ILOGENGINE_HPP

#include <string_view>

class ILogEngine
{
public:
	virtual void WPrint( const std::wstring_view msg ) = 0;
	virtual void WPrintln( const std::wstring_view msg ) = 0;

	virtual void Print( const std::string_view msg ) = 0;
	virtual void Println( const std::string_view msg ) = 0;
};

#endif // ILOGENGINE_HPP