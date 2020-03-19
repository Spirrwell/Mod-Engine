#include "stb_filesystem.hpp"
#include "vfile.hpp"

int read_stb( void *user, char *data, int size )
{
	VFile *file = reinterpret_cast< VFile* >( user );

	if ( !file )
		return 0;

	return ( int )file->read( data, size );
}

void skip_stb( void *user, int n )
{
	VFile *file = reinterpret_cast< VFile* >( user );

	if ( !file )
		return;

	file->seek( n, SEEK_CUR );
}

int eof_stb( void *user )
{
	VFile *file = reinterpret_cast< VFile* >( user );

	if ( !file )
		return 0;

	return ( file->eof() ) ? 1 : 0;
}

const stbi_io_callbacks callbacks_stb =
{
	&read_stb,
	&skip_stb,
	&eof_stb
};