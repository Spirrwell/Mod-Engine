#ifndef IINPUTSYSTEM_HPP
#define IINPUTSYSTEM_HPP

#include <string>

using ButtonIndex = std::size_t;
constexpr static const ButtonIndex INVALID_BUTTON_INDEX = std::numeric_limits< ButtonIndex >::max();

class IInputSystem
{
public:
	virtual ~IInputSystem() = default;

	virtual bool IsValidButton( ButtonIndex buttonIndex ) const = 0;
	virtual ButtonIndex CreateButton( const std::string &buttonName ) = 0;

	virtual ButtonIndex FindButton( const std::string &buttonName ) = 0;

	virtual float GetMouseXRel() const = 0;
	virtual float GetMouseYRel() const = 0;

	virtual bool IsButtonReleased( ButtonIndex buttonIndex ) = 0;
	virtual bool IsButtonPressed( ButtonIndex buttonIndex ) = 0;
	virtual bool IsButtonJustReleased( ButtonIndex buttonIndex ) = 0;
	virtual bool IsButtonJustPressed( ButtonIndex buttonIndex ) = 0;
};

#endif // IINPUTSYSTEM_HPP