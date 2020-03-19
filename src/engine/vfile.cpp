#include "vfile.hpp"
#include "vpk.hpp"

VFile::VFile( const std::filesystem::path &filename, const std::string &pathid, FileSystem *fileSystem )
{
	open( filename, pathid, fileSystem );
}

VFile::~VFile()
{
	close();
}

void VFile::close()
{
	if ( file )
	{
		fclose( file );
		file = nullptr;
	}
}

void VFile::open( const std::filesystem::path &filename, const std::string &pathid, FileSystem *fileSystem )
{
	FileSystem::FindResult findResult = fileSystem->FindFile( filename, pathid );

	if ( !findResult )
		return;

	if ( findResult.mountFindResult )
	{
		MountFileHandle mountFileHandle = findResult.searchPath->mount->OpenFile( findResult.mountFindResult.index );

		if ( mountFileHandle )
		{
			file = mountFileHandle.file;
			start = mountFileHandle.start;
			end = mountFileHandle.end;
		}
	}
	else
	{
		const std::filesystem::path abspath = findResult.searchPath->abspath / filename;
		file = fopen( abspath.string().c_str(), "rb" );

		fseek( file, 0, SEEK_END );
		end = static_cast< size_t >( ftell( file ) );

		fseek( file, 0, SEEK_SET );
	}
}

std::size_t VFile::read( char *buffer, std::size_t count )
{
	return read( buffer, 1, count );
}

std::size_t VFile::read( char *buffer, std::size_t size, std::size_t count )
{
	if ( !file )
		return 0;

	if ( eof() )
		return 0;

	long int pos = ftell( file );
	if ( pos == -1L )
		return 0;
	else if ( pos + count > end )
		count = end - pos;

	const size_t nRead = fread( buffer, size, count, file );

	if ( pos + nRead > end )
		iseof = true;

	return nRead;
}

bool VFile::seek( long int offset, int origin )
{
	if ( !file )
		return false;

	if ( origin == SEEK_SET )
	{
		offset += static_cast< long int >( start );

		if ( static_cast< size_t >( offset ) < start || static_cast< size_t >( offset ) > end )
			return false;

		return ( fseek( file, offset, SEEK_SET ) == 0 );
	}
	else if ( origin == SEEK_CUR )
	{
		long int pos = ftell( file );
		const long int seekpos = pos + offset;

		if ( static_cast< size_t >( seekpos ) >= start && static_cast< size_t >( seekpos ) <= end )
			return ( fseek( file, offset, SEEK_CUR ) == 0 );
		else
			return false;
	}
	else if ( origin == SEEK_END )
	{
		offset += static_cast< long int >( end );

		if ( static_cast< size_t >( offset ) < start || static_cast< size_t >( offset ) > end )
			return false;

		return ( fseek( file, offset, SEEK_SET ) == 0 );
	}

	return false;
}

bool VFile::eof() const
{
	return iseof;
}