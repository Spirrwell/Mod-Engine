#ifndef GAME_HPP
#define GAME_HPP

#include "game/igame.hpp"
#include "engine/imodel.hpp"
#include "engine/imodelsystem.hpp"
#include "engine/iresourcepool.hpp"
#include "camera.hpp"

class Game : public IGame
{
public:

	void configure( IEngine *engine ) override;
	void unconfigure( IEngine *engine ) override;

	void tick( float dt ) override;

	void UpdateFrameCount( float dt );
	void UpdateCamera();

	void InputCamera( float dt );

private:
	Camera camera;
	glm::vec3 position = {};
	glm::vec3 eulerAngles = {};

	IRenderSystem *renderer = nullptr;
	IInputSystem *inputSystem = nullptr;
	
	ButtonIndex Jump = INVALID_BUTTON_INDEX;
	ButtonIndex Crouch = INVALID_BUTTON_INDEX;
	ButtonIndex Forward = INVALID_BUTTON_INDEX;
	ButtonIndex Back = INVALID_BUTTON_INDEX;
	ButtonIndex Right = INVALID_BUTTON_INDEX;
	ButtonIndex Left = INVALID_BUTTON_INDEX;

	float frameClock = 0.0f;
	unsigned int frameCount = 0;

	IModelSystem *modelSystem = nullptr;
	IResourcePool *resourcePool = nullptr;
	IModel *dungeon = nullptr;
};

#endif // GAME_HPP