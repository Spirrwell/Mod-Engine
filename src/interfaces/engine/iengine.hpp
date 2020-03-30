#ifndef IENGINE_HPP
#define IENGINE_HPP

#include "engine/ilogengine.hpp"
#include "engine/irendersystem.hpp"
#include "engine/iinputsystem.hpp"
#include "engine/imodelsystem.hpp"
#include "engine/iresourcepool.hpp"

#include <string>
#include <string_view>

class IGame;

class IEngine : public ILogEngine
{
public:
	virtual ~IEngine() = default;

	[[noreturn]] virtual void Error( const std::string_view msg ) const = 0;

	virtual IGame *GetGame() const = 0;
	virtual void SetGame( IGame *game ) = 0;

	virtual IResourcePool *CreateResourcePool( IResourcePool *parent ) = 0;
	virtual void DestroyResourcePool( IResourcePool *resourcePoolPtr ) = 0;

	virtual IResourcePool *GetGlobalResourcePool() const = 0;

	virtual IRenderSystem *GetIRenderSystem() const = 0;
	virtual IInputSystem *GetIInputSystem() const = 0;
	virtual IModelSystem *GetIModelSystem() const = 0;
};

#endif // IENGINE_HPP