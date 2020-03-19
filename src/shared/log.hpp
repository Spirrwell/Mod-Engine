#ifndef LOG_HPP
#define LOG_HPP

#include "engine/ilogengine.hpp"

#include <string>
#include <string_view>
#include <mutex>

#include "fmt/format.h"
#include "fmt/color.h"

#include "color.hpp"

class Log
{
	static ILogEngine *s_pLogEngine;
	static std::mutex rainbowMutex;

	static fmt::color GetRainbowColor()
	{
		std::lock_guard< std::mutex > lock( rainbowMutex );

		static const float inc = 5.0f;
		static float hue = 0.0f - inc;

		hue += inc;

		if ( hue > 360.0f )
			hue -= 360.0f;

		fmt::color color = {};
		uint8_t *data = reinterpret_cast< uint8_t* >( &color );

		uint8_t &red = data[ 2 ];
		uint8_t &green = data[ 1 ];
		uint8_t &blue = data[ 0 ];

		HtoRGB( red, green, blue, hue );

		return color;
	}

public:
	static void configure( ILogEngine *logEngine );
	static void unconfigure();

	template < typename ... Args >
	static void WPrint( const std::wstring_view msg, Args &&... args )
	{
		if constexpr ( sizeof...( Args ) != 0 )
		{
			auto formattedMsg = fmt::format( msg, args... );
			s_pLogEngine->WPrint( formattedMsg );
		}
		else
		{
			s_pLogEngine->WPrint( msg );
		}
	}

	template < typename ... Args >
	static void WPrintColor( fmt::color color, const std::wstring_view msg, Args &&... args )
	{
		if constexpr ( sizeof...( Args ) != 0 )
		{
			auto formattedMsg = fmt::format( fmt::fg( color ), msg, args... );
			s_pLogEngine->WPrint( formattedMsg );
		}
		else
		{
			auto formattedMsg = fmt::format( fmt::fg( color ), msg );
			s_pLogEngine->WPrint( formattedMsg );
		}
	}

	template < typename ... Args >
	static void WPrintRainbow( const std::wstring_view msg, Args &&... args )
	{
		if constexpr ( sizeof...( Args ) != 0 )
		{
			auto formattedMsg = fmt::format( msg, args... );
			for ( size_t i = 0; i < formattedMsg.size(); ++i )
			{
				s_pLogEngine->WPrint( fmt::format( fmt::fg( GetRainbowColor() ), L"{}", formattedMsg[ i ] ) );
			}
		}
		else
		{
			for ( size_t i = 0; i < msg.size(); ++i )
			{
				s_pLogEngine->WPrint( fmt::format( fmt::fg( GetRainbowColor() ), L"{}", msg[ i ] ) );
			}
		}
	}

	template < typename ... Args >
	static void WPrintWarn( const std::wstring_view msg, Args &&... args )
	{
		if constexpr ( sizeof...( Args ) != 0 )
		{
			auto formattedMsg = fmt::format( msg, args... );
			WPrintColor( fmt::color::indian_red, formattedMsg );
		}
		else
		{
			WPrintColor( fmt::color::indian_red, msg );
		}
	}

	template < typename ... Args >
	static void WPrintln( const std::wstring_view msg, Args &&... args )
	{
		if constexpr ( sizeof...( Args ) != 0 )
		{
			auto formattedMsg = fmt::format( msg, args... );
			s_pLogEngine->WPrintln( formattedMsg );
		}
		else
		{
			s_pLogEngine->WPrintln( msg );
		}
	}

	template < typename ... Args >
	static void WPrintlnColor( fmt::color color, const std::wstring_view msg, Args &&... args )
	{
		if constexpr ( sizeof...( Args ) != 0 )
		{
			auto formattedMsg = fmt::format( fmt::fg( color ), msg, args... );
			s_pLogEngine->WPrintln( formattedMsg );
		}
		else
		{
			auto formattedMsg = fmt::format( fmt::fg( color ), msg );
			s_pLogEngine->WPrintln( formattedMsg );
		}
	}

	template < typename ... Args >
	static void WPrintlnRainbow( const std::wstring_view msg, Args &&... args )
	{
		if constexpr ( sizeof...( Args ) != 0 )
		{
			std::wstring formattedMsg = fmt::format( msg, args... );
			for ( size_t i = 0; i < formattedMsg.size(); ++i )
			{
				s_pLogEngine->WPrint( fmt::format( fmt::fg( GetRainbowColor() ), L"{}", formattedMsg[ i ] ) );
			}
		}
		else
		{
			for ( size_t i = 0; i < msg.size(); ++i )
			{
				s_pLogEngine->WPrint( fmt::format( fmt::fg( GetRainbowColor() ), L"{}", msg[ i ] ) );
			}
		}

		s_pLogEngine->WPrintln( L"" );
	}

	template < typename ... Args >
	static void WPrintlnWarn( const std::wstring_view msg, Args &&... args )
	{
		if constexpr ( sizeof...( Args ) != 0 )
		{
			auto formattedMsg = fmt::format( msg, args... );
			WPrintlnColor( fmt::color::indian_red, formattedMsg );
		}
		else
		{
			WPrintlnColor( fmt::color::indian_red, msg );
		}
	}

	template < typename ... Args >
	static void Print( const std::string_view msg, Args &&... args )
	{
		if constexpr ( sizeof...( Args ) != 0 )
		{
			auto formattedMsg = fmt::format( msg, args... );
			s_pLogEngine->Print( formattedMsg );
		}
		else
		{
			s_pLogEngine->Print( msg );
		}
	}

	template < typename ... Args >
	static void PrintColor( fmt::color color, const std::string_view msg, Args &&... args )
	{
		if constexpr ( sizeof...( Args ) != 0 )
		{
			auto formattedMsg = fmt::format( fmt::fg( color ), msg, args... );
			s_pLogEngine->Print( formattedMsg );
		}
		else
		{
			auto formattedMsg = fmt::format( fmt::fg( color ), msg );
			s_pLogEngine->Print( formattedMsg );
		}
	}

	template < typename ... Args >
	static void PrintRainbow( const std::string_view msg, Args &&... args )
	{
		if constexpr ( sizeof...( Args ) != 0 )
		{
			auto formattedMsg = fmt::format( msg, args... );
			for ( size_t i = 0; i < formattedMsg.size(); ++i )
			{
				s_pLogEngine->Print( fmt::format( fmt::fg( GetRainbowColor() ), "{}", formattedMsg[ i ] ) );
			}
		}
		else
		{
			for ( size_t i = 0; i < msg.size(); ++i )
			{
				s_pLogEngine->Print( fmt::format( fmt::fg( GetRainbowColor() ), "{}", msg[ i ] ) );
			}
		}
	}

	template < typename ... Args >
	static void PrintWarn( const std::string_view msg, Args &&... args )
	{
		if constexpr ( sizeof...( Args ) != 0 )
		{
			auto formattedMsg = fmt::format( msg, args... );
			PrintColor( fmt::color::indian_red, formattedMsg );
		}
		else
		{
			PrintColor( fmt::color::indian_red, msg );
		}
	}

	template < typename ... Args >
	static void Println( const std::string_view msg, Args &&... args )
	{
		if constexpr ( sizeof...( Args ) != 0 )
		{
			auto formattedMsg = fmt::format( msg, args... );
			s_pLogEngine->Println( formattedMsg );
		}
		else
		{
			s_pLogEngine->Println( msg );
		}
	}

	template < typename ... Args >
	static void PrintlnColor( fmt::color color, const std::string_view msg, Args &&... args )
	{
		if constexpr ( sizeof...( Args ) != 0 )
		{
			auto formattedMsg = fmt::format( fmt::fg( color ), msg, args... );
			s_pLogEngine->Println( formattedMsg );
		}
		else
		{
			auto formattedMsg = fmt::format( fmt::fg( color ), msg );
			s_pLogEngine->Println( formattedMsg );
		}
	}

	template < typename ... Args >
	static void PrintlnRainbow( const std::string_view msg, Args &&... args )
	{
		if constexpr ( sizeof...( Args ) != 0 )
		{
			auto formattedMsg = fmt::format( msg, args... );
			for ( size_t i = 0; i < formattedMsg.size(); ++i )
			{
				s_pLogEngine->Print( fmt::format( fmt::fg( GetRainbowColor() ), "{}", formattedMsg[ i ] ) );
			}
		}
		else
		{
			for ( size_t i = 0; i < msg.size(); ++i )
			{
				s_pLogEngine->Print( fmt::format( fmt::fg( GetRainbowColor() ), "{}", msg[ i ] ) );
			}
		}

		s_pLogEngine->Println( "" );
	}

	template < typename ... Args >
	static void PrintlnWarn( const std::string_view msg, Args &&... args )
	{
		if constexpr ( sizeof...( Args ) != 0 )
		{
			auto formattedMsg = fmt::format( msg, args... );
			PrintlnColor( fmt::color::indian_red, formattedMsg );
		}
		else
		{
			PrintlnColor( fmt::color::indian_red, msg );
		}
	}
};

#endif // LOG_HPP