#include "inputsystem.hpp"
#include "engine.hpp"
#include "log.hpp"

void InputSystem::configure( Engine *engine )
{
	EngineSystem::configure( engine );

	Jump = CreateButton( "Jump" );
	SetKeyBinding( Jump, { SDLK_SPACE } );

	Crouch = CreateButton( "Crouch" );
	SetKeyBinding( Crouch, { SDLK_LCTRL } );

	Quit = CreateButton( "Quit" );
	SetKeyBinding( Quit, { SDLK_ESCAPE } );
	SetJoyBinding( Quit, { -1, SDL_CONTROLLER_BUTTON_START } );

	Forward = CreateButton( "Forward" );
	SetKeyBinding( Forward, { SDLK_w } );

	Back = CreateButton( "Back" );
	SetKeyBinding( Back, { SDLK_s } );

	Right = CreateButton( "Right" );
	SetKeyBinding( Right, { SDLK_d } );

	Left = CreateButton( "Left" );
	SetKeyBinding( Left, { SDLK_a } );

	renderSystem = engine->GetRenderSystem();
}

void InputSystem::unconfigure( Engine *engine )
{
	renderSystem = nullptr;

	for ( auto pGameController : gameControllers )
		SDL_GameControllerClose( pGameController );

	gameControllers.clear();

	for ( auto pButtonCode : buttonCodes )
		delete pButtonCode;

	buttonCodes.clear();
	inputType = InputType::KeyboardAndMouse;

	EngineSystem::unconfigure( engine );
}

void InputSystem::HandleSDLEvents()
{
	static SDL_Event event;

	for ( auto pButton : buttonCodes )
		pButton->isPressed.SetOldToNow();

	mouseXRel = 0.0f;
	mouseYRel = 0.0f;

	while ( SDL_PollEvent( &event ) )
	{
		switch ( event.type )
		{
			case SDL_QUIT:
			{
				engine->Quit();
				break;
			}
			case SDL_WINDOWEVENT:
			{
				if ( event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED ) {
					renderSystem->NotifyWindowResized( static_cast< uint32_t >( event.window.data1 ), static_cast< uint32_t >( event.window.data2 ) );
				}
				else if ( event.window.event == SDL_WINDOWEVENT_MINIMIZED ) {
					renderSystem->NotifyWindowMinimized();
				}
				else if ( event.window.event == SDL_WINDOWEVENT_MAXIMIZED || event.window.event == SDL_WINDOWEVENT_RESTORED ) {
					renderSystem->NotifyWindowMaximized();
				}

				break;
			}
			case SDL_DROPFILE:
			{
				SDL_free( event.drop.file );
				break;
			}
			case SDL_MOUSEMOTION:
			{
				mouseXRel = static_cast< float >( event.motion.xrel );
				mouseYRel = static_cast< float >( event.motion.yrel );
				break;
			}
			case SDL_KEYDOWN:
			{
				const SDL_Keycode key = event.key.keysym.sym;
				for ( auto pButton : buttonCodes )
				{
					if ( pButton->keyBinding.key == key ) {
						pButton->isPressed.now = true;
					}
				}

				break;
			}
			case SDL_KEYUP:
			{
				const SDL_Keycode key = event.key.keysym.sym;
				for ( auto pButton : buttonCodes )
				{
					if ( pButton->keyBinding.key == key ) {
						pButton->isPressed.now = false;
					}
				}

				break;
			}
			case SDL_CONTROLLERDEVICEADDED:
			{
				Log::Println( "ControllerDeviceAdded: {}", event.cdevice.which );
				gameControllers.push_back( SDL_GameControllerOpen( event.cdevice.which ) );
				
				break;
			}
			case SDL_CONTROLLERDEVICEREMOVED:
			{
				Log::Println( "ControllerDeviceRemoved: {}", event.cdevice.which );
				break;
			}
			case SDL_CONTROLLERBUTTONDOWN:
			{
				//SetInputType( InputType::Controller );
				const Uint8 button = event.cbutton.button;

				Log::Println( "ControllerButtonDown: {}", button );

				for ( auto pButton : buttonCodes )
				{
					if ( pButton->controllerBinding.controllerButton == button ) {
						pButton->isPressed.now = true;
					}
				}

				break;
			}
			case SDL_CONTROLLERBUTTONUP:
			{
				//SetInputType( InputType::Controller );
				const Uint8 button = event.cbutton.button;

				for ( auto pButton : buttonCodes )
				{
					if ( pButton->controllerBinding.controllerButton == button ) {
						pButton->isPressed.now = false;
					}
				}

				break;
			}
		}
	}

	if ( IsButtonPressed( Quit ) )
		engine->Quit();
}

bool InputSystem::IsValidButton( ButtonIndex buttonIndex ) const
{
	if ( buttonIndex == INVALID_BUTTON_INDEX )
		return false;

	return ( buttonIndex < buttonCodes.size() );
}

ButtonIndex InputSystem::CreateButton( const std::string &buttonName )
{
	ButtonCode *buttonCode = new ButtonCode;
	buttonCode->buttonName = buttonName;
	buttonCode->buttonIndex = buttonCodes.size();
	buttonCodes.push_back( buttonCode );

	return buttonCode->buttonIndex;
}

ButtonIndex InputSystem::FindButton( const std::string &buttonName )
{
	for ( auto buttonCode : buttonCodes )
	{
		if ( buttonCode->buttonName == buttonName )
			return buttonCode->buttonIndex;
	}

	return INVALID_BUTTON_INDEX;
}

void InputSystem::SetJoyBinding( ButtonIndex buttonIndex, ControllerBinding controllerBinding )
{
	if ( IsValidButton( buttonIndex ) ) {
		buttonCodes[ buttonIndex ]->controllerBinding = controllerBinding;
	}
}

void InputSystem::SetKeyBinding( ButtonIndex buttonIndex, KeyboardBinding keyBinding )
{
	if ( IsValidButton( buttonIndex ) ) {
		buttonCodes[ buttonIndex ]->keyBinding = keyBinding;
	}
}

bool InputSystem::IsButtonReleased( ButtonIndex buttonIndex )
{
	if ( IsValidButton( buttonIndex ) ) {
		return !buttonCodes[ buttonIndex ]->isPressed.now;
	}

	return true;
}

bool InputSystem::IsButtonPressed( ButtonIndex buttonIndex )
{
	if ( IsValidButton( buttonIndex ) ) {
		return buttonCodes[ buttonIndex ]->isPressed.now;
	}

	return false;
}

bool InputSystem::IsButtonJustReleased( ButtonIndex buttonIndex )
{
	if ( IsValidButton( buttonIndex ) ) {
		return ( !buttonCodes[ buttonIndex ]->isPressed.now && buttonCodes[ buttonIndex ]->isPressed.old );
	}

	return false;
}

bool InputSystem::IsButtonJustPressed( ButtonIndex buttonIndex )
{
	if ( IsValidButton( buttonIndex ) ) {
		return ( buttonCodes[ buttonIndex ]->isPressed.now && !buttonCodes[ buttonIndex ]->isPressed.old );
	}

	return false;
}