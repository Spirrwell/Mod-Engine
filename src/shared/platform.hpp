#ifndef PLATFORM_HPP
#define PLATFORM_HPP

#if defined ( _MSC_VER )
#define EXPORT __declspec( dllexport )
#define IMPORT __declspec( dllimport )
#elif defined ( __GNUC__ )
#define EXPORT __attribute__( ( visibility ( "default" ) ) )
#define IMPORT
#endif

#endif // PLATFORM_HPP