#ifndef INPUTEVENT_HPP
#define INPUTEVENT_HPP

#include <SDL2/SDL.h>

// These events mirror SDL's input events
namespace InputEvent
{
	namespace Keyboard
	{
		struct KeyDown
		{
			const SDL_KeyboardEvent key;
		};

		struct KeyUp
		{
			const SDL_KeyboardEvent key;
		};
	}
		
	namespace Mouse
	{
		struct Motion
		{
			const SDL_MouseMotionEvent motion;
		};

		struct ButtonDown
		{
			const SDL_MouseButtonEvent button;
		};

		struct ButtonUp
		{
			const SDL_MouseButtonEvent button;
		};

		struct Wheel
		{
			const SDL_MouseWheelEvent wheel;
		};
	}

	namespace Joystick
	{
		struct AxisMotion
		{
			const SDL_JoyAxisEvent jaxis;
		};

		struct BallMotion
		{
			const SDL_JoyBallEvent jball;
		};

		struct HatMotion
		{
			const SDL_JoyHatEvent jhat;
		};

		struct ButtonDown
		{
			const SDL_JoyButtonEvent jbutton;
		};

		struct ButtonUp
		{
			const SDL_JoyButtonEvent jbutton;
		};

		struct DeviceAdded
		{
			const SDL_JoyDeviceEvent jdevice;
		};

		struct DeviceRemoved
		{
			const SDL_JoyDeviceEvent jdevice;
		};
	}

	namespace GameController
	{
		struct AxisMotion
		{
			const SDL_ControllerAxisEvent caxis;
		};

		struct ButtonDown
		{
			const SDL_ControllerButtonEvent cbutton;
		};

		struct ButtonUp
		{
			const SDL_ControllerButtonEvent cbutton;
		};

		struct DeviceAdded
		{
			const SDL_ControllerDeviceEvent cdevice;
		};

		struct DeviceRemoved
		{
			const SDL_ControllerDeviceEvent cdevice;
		};

		struct DeviceRemapped
		{
			const SDL_ControllerDeviceEvent cdevice;
		};
	}
}

#endif // INPUTEVENT_HPP