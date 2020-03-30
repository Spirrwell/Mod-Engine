#include "game.hpp"
#include "platform.hpp"
#include "log.hpp"

#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"

void Game::configure( IEngine *engine )
{
	renderSystem = engine->GetIRenderSystem();
	inputSystem = engine->GetIInputSystem();
	modelSystem = engine->GetIModelSystem();

	Jump = inputSystem->FindButton( "Jump" );
	Crouch = inputSystem->FindButton( "Crouch" );
	Forward = inputSystem->FindButton( "Forward" );
	Back = inputSystem->FindButton( "Back" );
	Right = inputSystem->FindButton( "Right" );
	Left = inputSystem->FindButton( "Left" );

	resourcePool = engine->CreateResourcePool( engine->GetGlobalResourcePool() );
	dungeon = modelSystem->LoadModel( "models/Mazmorra.obj", "GAME", resourcePool );
}

void Game::unconfigure( IEngine *engine )
{
	engine->DestroyResourcePool( resourcePool );
	inputSystem = nullptr;
}

void Game::tick( float dt )
{
	UpdateFrameCount( dt );
	UpdateCamera();
	InputCamera( dt );

	glm::mat4 modelMat = glm::mat4( 1.0f );
	modelMat = glm::translate( modelMat, glm::vec3( 0.0f, 0.0f, 0.0f ) );
	renderSystem->DrawModel( dungeon, modelMat );
}

void Game::UpdateFrameCount( float dt )
{
	++frameCount;
	frameClock += dt;

	if ( frameClock >= 1.0f )
	{
		Log::PrintlnRainbow( "FPS: {}", frameCount );
		frameClock = 0.0f;
		frameCount = 0;
	}
}

void Game::UpdateCamera()
{
	auto &pitch = eulerAngles[ 0 ];
	auto &yaw = eulerAngles[ 1 ];
	auto &roll = eulerAngles[ 2 ];

	auto &viewMatrix = renderSystem->GetRenderView().viewMatrix;
	viewMatrix = glm::mat4( 1.0f );

	// Y Rotation
	viewMatrix = glm::rotate( viewMatrix, glm::radians( yaw ), glm::vec3( 0.0f, 1.0f, 0.0f ) );

	// X Rotation
	viewMatrix = glm::rotate( viewMatrix, glm::radians( pitch ), glm::vec3( viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0] ) );

	// Z Rotation
	viewMatrix = glm::rotate( viewMatrix, glm::radians( roll ), glm::vec3( viewMatrix[0][2], viewMatrix[1][2], viewMatrix[2][2] ) );

	// Translation
	viewMatrix = glm::translate( viewMatrix, -position );

	camera.forward = -glm::normalize( glm::vec3( viewMatrix[0][2], viewMatrix[1][2], viewMatrix[2][2] ) );
	camera.up = glm::normalize( glm::vec3( viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1] ) );
	camera.right = glm::normalize( glm::vec3( viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0] ) );
	camera.back = -camera.forward;
	camera.down = -camera.up;
	camera.left = -camera.right;
}

void Game::InputCamera( float dt )
{
	const float mouseXRel = inputSystem->GetMouseXRel();
	const float mouseYRel = inputSystem->GetMouseYRel();

	auto &pitch = eulerAngles[ 0 ];
	auto &yaw = eulerAngles[ 1 ];

	yaw += glm::degrees( mouseXRel * dt );
	pitch -= glm::degrees( mouseYRel * dt );

	const float speed = 5.0f;

	if ( inputSystem->IsButtonPressed( Forward ) )
		position += camera.forward * speed * dt;
	else if ( inputSystem->IsButtonPressed( Back ) )
		position += camera.back * speed * dt;

	if ( inputSystem->IsButtonPressed( Right ) )
		position += camera.right * speed * dt;
	else if ( inputSystem->IsButtonPressed( Left ) )
		position += camera.left * speed * dt;

	if ( inputSystem->IsButtonPressed( Jump ) )
		position += glm::vec3( 0.0f, -1.0f, 0.0f ) * speed * dt;
	else if ( inputSystem->IsButtonPressed( Crouch ) )
		position += glm::vec3( 0.0f, 1.0f, 0.0f ) * speed * dt;
}