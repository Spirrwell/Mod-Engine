#ifndef INPUTSYSTEM_HPP
#define INPUTSYSTEM_HPP

#include <string>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <array>

#include <SDL2/SDL.h>

#include "engine/iinputsystem.hpp"
#include "enginesystem.hpp"
#include "inputevent.hpp"
#include "memory.hpp"
#include "state.hpp"

class Renderer;

enum class InputType
{
	KeyboardAndMouse,
	Controller
};

struct KeyboardBinding
{
	SDL_Keycode key = SDLK_UNKNOWN;
};

struct MouseBinding
{
	Uint8 mouseButton = 0;
};

struct ControllerBinding
{
	SDL_JoystickID which = -1;
	Uint8 controllerButton = SDL_CONTROLLER_BUTTON_INVALID;
};

struct ButtonCode
{
	std::string buttonName;
	ButtonIndex buttonIndex;
	State< bool > isPressed;

	KeyboardBinding keyBinding;
	MouseBinding mouseBinding;
	ControllerBinding controllerBinding;
};

class InputSystem : public IInputSystem, public EngineSystem
{
public:

	void configure( Engine *engine ) override;
	void unconfigure( Engine *engine ) override;

	void HandleSDLEvents();

	bool IsValidButton( ButtonIndex buttonIndex ) const override;
	ButtonIndex CreateButton( const std::string &buttonName ) override;

	ButtonIndex FindButton( const std::string &buttonName ) override;

	void SetJoyBinding( ButtonIndex buttonIndex, ControllerBinding controllerBinding );
	void SetKeyBinding( ButtonIndex buttonIndex, KeyboardBinding keyBinding );

	InputType GetInputType() const { return inputType; }
	void SetInputType( InputType inputType ) { this->inputType = inputType; }

	float GetMouseXRel() const { return mouseXRel; }
	float GetMouseYRel() const { return mouseYRel; }

	bool IsButtonReleased( ButtonIndex buttonIndex ) override;
	bool IsButtonPressed( ButtonIndex buttonIndex ) override;
	bool IsButtonJustReleased( ButtonIndex buttonIndex ) override;
	bool IsButtonJustPressed( ButtonIndex buttonIndex ) override;

private:
	Renderer *renderer = nullptr;

	std::vector< SDL_GameController* > gameControllers;
	std::vector< ButtonCode* > buttonCodes;
	InputType inputType = InputType::KeyboardAndMouse;

	float mouseXRel = 0.0f;
	float mouseYRel = 0.0f;

	ButtonIndex Jump = INVALID_BUTTON_INDEX;
	ButtonIndex Crouch = INVALID_BUTTON_INDEX;
	ButtonIndex Quit = INVALID_BUTTON_INDEX;
	ButtonIndex Forward = INVALID_BUTTON_INDEX;
	ButtonIndex Back = INVALID_BUTTON_INDEX;
	ButtonIndex Right = INVALID_BUTTON_INDEX;
	ButtonIndex Left = INVALID_BUTTON_INDEX;
};

#endif // INPUTSYSTEM_HPP