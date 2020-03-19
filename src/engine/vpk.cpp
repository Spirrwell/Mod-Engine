#include "vpk.hpp"
#include "log.hpp"

#include <algorithm>

VPK::VPK( const std::filesystem::path &directoryPath ) :
	directoryPath( directoryPath )
{
	auto readString = []( std::string &buffer, std::ifstream &file )
	{
		while ( true )
		{
			char c = '\0';
			file.read( &c, 1 );

			if ( c == '\0' ) {
				break;
			}

			buffer.push_back( c );
		}
	};

	auto isEmptyBasePath = []( const std::string &base_path )
	{
		if ( base_path[ 0 ] == ' ' && base_path[ 1 ] == '\0' ) {
			return true;
		}

		return false;
	};

	if ( !std::filesystem::exists( directoryPath ) ) {
		Log::PrintlnWarn( "{} does not exist", directoryPath.string() );
		return;
	}

	std::unique_ptr< std::ifstream > file = make_unique< std::ifstream >( directoryPath, std::ios_base::binary );

	file->read( ( char* )&vpkHeader.Signature, sizeof( vpkHeader.Signature ) );
	file->read( ( char* )&vpkHeader.Version, sizeof( vpkHeader.Version ) );
	file->read( ( char* )&vpkHeader.TreeSize, sizeof( vpkHeader.TreeSize ) );

	if ( vpkHeader.Signature != VPK_SIGNATURE || vpkHeader.GetVersionSize() == 0 ) {
		Log::PrintlnWarn( "{} is not a valid VPK file", directoryPath.string() );
		return;
	}

	if ( vpkHeader.Version == 2 ) {
		file->read( ( char* )&vpkHeader.FileDataSectionSize, sizeof( vpkHeader.FileDataSectionSize ) );
		file->read( ( char* )&vpkHeader.ArchiveMD5SectionSize, sizeof( vpkHeader.ArchiveMD5SectionSize ) );
		file->read( ( char* )&vpkHeader.OtherMD5SectionSize, sizeof( vpkHeader.OtherMD5SectionSize ) );
		file->read( ( char* )&vpkHeader.SignatureSectionSize, sizeof( vpkHeader.SignatureSectionSize ) );
	}

	while ( true )
	{
		std::string extension;
		readString( extension, *file );

		if ( extension.empty() ) {
			break;
		}

		while ( true )
		{
			std::string base_path;
			readString( base_path, *file );

			if ( base_path.empty() ) {
				break;
			}

			while ( true )
			{
				std::string file_name;
				readString( file_name, *file );

				if ( file_name.empty() ) {
					break;
				}

				VPKFileEntry fileEntry = {};
				
				file->read( ( char* )&fileEntry.directoryEntry.CRC, sizeof( fileEntry.directoryEntry.CRC ) );
				file->read( ( char* )&fileEntry.directoryEntry.PreloadBytes, sizeof( fileEntry.directoryEntry.PreloadBytes ) );
				file->read( ( char* )&fileEntry.directoryEntry.ArchiveIndex, sizeof( fileEntry.directoryEntry.ArchiveIndex ) );
				file->read( ( char* )&fileEntry.directoryEntry.EntryOffset, sizeof( fileEntry.directoryEntry.EntryOffset ) );
				file->read( ( char* )&fileEntry.directoryEntry.EntryLength, sizeof( fileEntry.directoryEntry.EntryLength ) );
				file->read( ( char* )&fileEntry.directoryEntry.Terminator, sizeof( fileEntry.directoryEntry.Terminator ) );

				if ( fileEntry.directoryEntry.PreloadBytes )
				{
					Log::PrintlnWarn( "Ignoring preload data" );
					file->seekg( fileEntry.directoryEntry.PreloadBytes, std::ios_base::cur );
				}
				else
				{
					using namespace std::string_literals;

					if ( isEmptyBasePath( base_path ) )
						fileEntry.filename = fmt::format( "{}.{}", file_name, extension );
					else
					{
						std::replace( base_path.begin(), base_path.end(), '\\', '/' );
						fileEntry.filename = fmt::format( "{}/{}.{}", base_path, file_name, extension );
					}

					fileEntries.push_back( fileEntry );
				}
			}
		}
	}

	/*Log::PrintlnRainbow( "Printing VPK file entries" );
	for ( auto &fileEntry : fileEntries )
	{
		Log::PrintlnRainbow( "{}", fileEntry.filename.string() );
	}
	Log::PrintlnRainbow( "Done" );*/

	std::string base_vpkpath = directoryPath.string();
	std::string directoryPathLower = base_vpkpath;

	std::transform( directoryPathLower.begin(), directoryPathLower.end(), directoryPathLower.begin(), ::tolower );
	const std::size_t pos = directoryPathLower.find_last_of( "_dir.vpk" );

	if ( pos != std::string::npos )
	{
		base_vpkpath.erase( pos );
		int index = 0;

		while ( true )
		{
			const std::string archivePath = fmt::format( "{}_{:03d}.vpk", base_vpkpath, index );
			if ( !std::filesystem::exists( archivePath ) )
				break;

			archiveInfos.push_back( {} );

			ArchiveInfo &archiveInfo = archiveInfos.back();
			archiveInfo.file = make_unique< std::ifstream >( archivePath );
			archiveInfo.archivePath = archivePath;

			++index;
		}
	}

	directoryFile = std::move( file );
}

MountFileHandle VPK::OpenFile( std::size_t index )
{
	FILE *file = nullptr;
	auto handleError = [ &file ]()
	{
		if ( file )
		{
			fclose( file );
			file = nullptr;
		}
	};

	auto &fileEntry = fileEntries[ index ];
	const uint16_t archiveIndex = fileEntry.directoryEntry.ArchiveIndex;

	if ( isArchive( archiveIndex ) )
	{
		if ( archiveIndex >= archiveInfos.size() )
			return {};
		else
		{
			file = fopen( archiveInfos[ archiveIndex ].archivePath.string().c_str(), "rb" );

			if ( !file )
				return {};

			const uint32_t start = fileEntry.directoryEntry.EntryOffset;
			if ( fseek( file, start, SEEK_SET ) != 0 )
			{
				handleError();
				return {};
			}

			return MountFileHandle 
			{
				file,
				static_cast< size_t >( start ),
				static_cast< size_t >( start + fileEntry.directoryEntry.EntryLength )
			};
		}
	}

	file = fopen( directoryPath.string().c_str(), "rb" );
	
	if ( !file )
		return {};

	const uint32_t start = vpkHeader.GetVersionSize() + vpkHeader.TreeSize + fileEntry.directoryEntry.EntryOffset;

	if ( fseek( file, start, SEEK_SET ) != 0 )
		handleError();

	return MountFileHandle 
	{
		file,
		static_cast< size_t >( start ),
		static_cast< size_t >( start + fileEntry.directoryEntry.EntryLength )
	};
}

FileSystem::MountFindResult VPK::FindFile( const std::filesystem::path &relpath ) const
{
	FileSystem::MountFindResult result;

	for ( size_t i = 0; i < fileEntries.size(); ++i )
	{
		const auto &entry = fileEntries[ i ];
		if ( entry.filename == relpath )
		{
			result.index = i;
			break;
		}
	}
	
	return result;
}

bool VPK::ReadToBuffer( std::vector< char > &buffer, size_t index )
{
	MountFileHandle mountFileHandle = OpenFile( index );

	if ( !mountFileHandle )
		return false;

	buffer.resize( fileEntries[ index ].directoryEntry.EntryLength );
	const bool success = fread( buffer.data(), 1, buffer.size(), mountFileHandle.file ) == buffer.size();
	fclose( mountFileHandle.file );

	if ( !success )
		buffer.clear();

	return success;
}