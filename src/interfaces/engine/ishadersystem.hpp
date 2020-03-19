#ifndef ISHADERSYSTEM_HPP
#define ISHADERSYSTEM_HPP

#include <filesystem>
#include <string>

#include "engine/ishader.hpp"
#include "engine/iresourcepool.hpp"

class IShaderSystem
{
public:
	virtual ~IShaderSystem() = default;

	virtual IShader *FindShader( const std::string &shaderName ) const = 0;
};

#endif // ISHADERSYSTEM_HPP