#ifndef STB_FILESYSTEM_HPP
#define STB_FILESYSTEM_HPP

#include "stb_image.h"

int read_stb( void *user, char *data, int size );	// fill 'data' with 'size' bytes.  return number of bytes actually read
void skip_stb( void *user, int n );					// skip the next 'n' bytes, or 'unget' the last -n bytes if negative
int eof_stb( void *user );							// returns nonzero if we are at end of file/data

extern const stbi_io_callbacks callbacks_stb;

#endif