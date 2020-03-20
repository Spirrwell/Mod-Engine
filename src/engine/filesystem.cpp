#include "filesystem.hpp"
#include "engine.hpp"
#include "log.hpp"
#include "vpk.hpp"

#include <fstream>

void FileSystem::configure( Engine *engine )
{
	EngineSystem::configure( engine );

	CommandLineSystem *commandlineSystem = engine->GetCommandLineSystem();
	const auto game = commandlineSystem->GetArgumentInput( "-game" );

	if ( game.empty() )
		engine->Error( "No -game specified!" );

	gameDir = game[ 0 ];
	gameBinDir = gameDir / "bin";

	AddSearchPath( gameDir, "GAME" );
	//AddSearchPathVPK( "mod_quakelike.vpk", "GAME" );
}

void FileSystem::unconfigure( Engine *engine )
{
	for ( auto kv : searchPaths )
	{
		for ( auto searchPath : kv.second )
			delete searchPath;
	}

	searchPaths.clear();

	EngineSystem::unconfigure( engine );
}

bool FileSystem::ReadToBuffer( const std::filesystem::path &relpath, const std::string &pathid, std::vector< char > &buffer )
{
	FindResult findResult = FindFile( relpath, pathid );

	if ( findResult )
	{
		if ( findResult.mountFindResult )
		{
			return findResult.searchPath->mount->ReadToBuffer( buffer, findResult.mountFindResult.index );
		}
		else
		{
			const std::filesystem::path abspath = findResult.searchPath->abspath / relpath;
			std::ifstream file( abspath, std::ios_base::binary | std::ios_base::ate );

			auto fileSize = file.tellg();
			buffer.resize( fileSize );

			file.seekg( 0 );

			if ( file.read( buffer.data(), buffer.size() ).gcount() < static_cast< std::streamsize >( buffer.size() ) )
			{
				buffer.clear();
				return false;
			}

			return true;
		}
	}

	return false;
}

bool FileSystem::Exists( const std::filesystem::path &relpath ) const
{
	return ( bool )FindFile( relpath, "" );
}
bool FileSystem::Exists( const std::filesystem::path &relpath, const std::string &pathid ) const
{
	return ( bool )FindFile( relpath, pathid );
}

FileSystem::FindResult FileSystem::FindFile( const std::filesystem::path &relpath, const std::string &pathid ) const
{
	FindResult result;

	auto processSearchPaths = [ &result, &relpath ]( const std::vector< FSearchPath* > &searchPaths )
	{
		// This MUST be by reference since we're using a unique_ptr
		for ( auto &searchPath : searchPaths )
		{
			if ( searchPath->isMount() )
			{
				MountFindResult mountFindResult = {};
				switch ( searchPath->mount->GetCaseSensitivity() )
				{
					case Mount::CaseSensitivity::Sensitive:
					{
						mountFindResult = searchPath->mount->FindFile( relpath );
						break;
					}
					case Mount::CaseSensitivity::Lower:
					{
						std::string relpath_lower = relpath.generic_string();
						std::transform( relpath_lower.begin(), relpath_lower.end(), relpath_lower.begin(), ::tolower );

						mountFindResult = searchPath->mount->FindFile( relpath_lower );
						break;
					}
					case Mount::CaseSensitivity::Upper:
					{
						std::string relpath_upper = relpath.generic_string();
						std::transform( relpath_upper.begin(), relpath_upper.end(), relpath_upper.begin(), ::toupper );

						mountFindResult = searchPath->mount->FindFile( relpath_upper );
						break;
					}
				}

				if ( mountFindResult )
				{
					result.searchPath = searchPath;
					result.relpath = relpath;
					result.mountFindResult = mountFindResult;
				}
			}
			else
			{
				std::filesystem::path absfilename = searchPath->abspath / relpath;
				if ( std::filesystem::exists( absfilename ) )
				{
					result.searchPath = searchPath;
					result.relpath = relpath;
				}
			}

			if ( result )
				break;
		}
	};

	// If this is an absolute path, you shouldn't call us
	if ( relpath.is_relative() )
	{
		if ( pathid.empty() )
		{
			auto allPaths = GetAllUniqueSearchPaths();
			processSearchPaths( allPaths );
		}
		else
		{
			if ( auto it = searchPaths.find( pathid ); it != searchPaths.end() )
				processSearchPaths( it->second );
		}
	}

	return result;
}


void FileSystem::AddSearchPath( const std::filesystem::path &path, const std::string &pathid )
{
	if ( pathid.empty() )
		return;

	std::filesystem::path abspath = std::filesystem::absolute( path );

	FSearchPath *searchPath = new FSearchPath;
	searchPath->abspath = abspath;
	searchPath->pathid = pathid;

	searchPaths[ pathid ].push_back( searchPath );
}

void FileSystem::AddSearchPathVPK( const std::filesystem::path &vpkpath, const std::string &pathid )
{
	if ( pathid.empty() )
		return;

	std::filesystem::path abspath = std::filesystem::absolute( vpkpath );
	unique_ptr< Mount > vpk = LoadVPK( abspath );

	if ( vpk ) {
		FSearchPath *searchPath = new FSearchPath;
		searchPath->abspath = abspath;
		searchPath->pathid = pathid;
		searchPath->mount = std::move( vpk );

		searchPaths[ pathid ].push_back( searchPath );
	}
}

std::vector< FileSystem::FSearchPath* > FileSystem::GetAllUniqueSearchPaths() const
{
	std::unordered_map< FSearchPath*, bool > visitedPaths;
	std::vector< FSearchPath* > uniquePaths;

	for ( auto &kv : searchPaths )
	{
		for ( auto &searchPath : kv.second )
		{
			if ( visitedPaths[ searchPath ] )
				continue;

			visitedPaths[ searchPath ] = true;
			uniquePaths.push_back( searchPath );
		}
	}

	return uniquePaths;
}

unique_ptr< Mount > FileSystem::LoadVPK( const std::filesystem::path &path )
{
	Log::Println( "Loading VPK {}", path.generic_string() );
	unique_ptr< Mount > vpk = make_unique< VPK >( path );

	if ( vpk->IsValid() )
		return vpk;

	engine->Error( fmt::format( "Failed to load VPK {}", path.generic_string() ) );

	return nullptr;
}