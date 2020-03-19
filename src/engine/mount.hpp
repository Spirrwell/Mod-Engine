#ifndef MOUNT_HPP
#define MOUNT_HPP

#include "filesystem.hpp"

#include <vector>

class Mount
{
public:
	virtual ~Mount() = default;

	enum class CaseSensitivity
	{
		Sensitive,
		Lower,
		Upper
	};

	virtual bool IsValid() const = 0;

	virtual CaseSensitivity GetCaseSensitivity() const = 0;

	virtual MountFileHandle OpenFile( std::size_t index ) = 0;
	virtual FileSystem::MountFindResult FindFile( const std::filesystem::path &filename ) const = 0;

	virtual bool ReadToBuffer( std::vector< char > &buffer, std::size_t index ) = 0;
};

#endif // MOUNT_HPP