#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include <filesystem>

#include "memory.hpp"
#include "enginesystem.hpp"
#include "commandlinesystem.hpp"

#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <limits>

class Mount;

struct MountFileHandle
{
	operator bool() const { return is_valid(); }
	bool is_valid() const { return ( file ); }

	FILE *file = nullptr;
	size_t start = 0;
	size_t end = 0;
};

class FileSystem : public EngineSystem
{
public:

	void configure( Engine *engine ) override;
	void unconfigure( Engine *engine ) override;

	std::filesystem::path GetGameDir() const { return gameDir; }
	std::filesystem::path GetGameBinDir() const { return gameBinDir; }

	// This is FSearchPath in case we include windows headers which has a 'SearchPath' definition yuck
	struct FSearchPath
	{
		bool isMount() const { return mount.get() != nullptr; }

		std::filesystem::path abspath;
		std::string pathid;
		unique_ptr< Mount > mount;
	};

	struct MountFindResult
	{
		operator bool() const { return is_valid(); }
		bool is_valid() const { return ( index != InvalidIndex() ); }

		size_t index = InvalidIndex();

		static size_t constexpr InvalidIndex() { return std::numeric_limits< size_t >::max(); }
	};

	struct FindResult
	{
		operator bool() const { return is_valid(); }
		bool is_valid() const { return ( searchPath && !relpath.empty() ); }

		FSearchPath *searchPath = nullptr;
		std::filesystem::path relpath;
		MountFindResult mountFindResult;
	};

	// Reads the entire contents of a file to a buffer, returns false on failure and 'buffer' will be emptied
	bool ReadToBuffer( const std::filesystem::path &relpath, const std::string &pathid, std::vector< char > &buffer );
	
	// Cheks if a file exists in our filesystem given a relative path
	bool Exists( const std::filesystem::path &relpath ) const;
	bool Exists( const std::filesystem::path &relpath, const std::string &pathid ) const;

	// Attempts to find an existing file in our filesystem and returns a "FindResult" with information about our findings given a relative path
	FindResult FindFile( const std::filesystem::path &relpath, const std::string &pathid ) const;

	// Mounts paths as search paths when looking up files in our filesystem
	void AddSearchPath( const std::filesystem::path &path, const std::string &pathid );
	void AddSearchPathVPK( const std::filesystem::path &vpkpath, const std::string &pathid );

private:
	std::vector< FSearchPath* > GetAllUniqueSearchPaths() const;
	unique_ptr< Mount > LoadVPK( const std::filesystem::path &path );

	std::unordered_map< std::string, std::vector< FSearchPath* > > searchPaths; // Maps path id to a search path

	std::filesystem::path gameDir;
	std::filesystem::path gameBinDir;
};

#endif // FILESYSTEM_HPP