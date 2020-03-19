#ifndef VFILE_HPP
#define VFILE_HPP

#include "memory.hpp"
#include "filesystem.hpp"

#include <cstdio>
#include <fstream>
#include <string>
#include <cstdint>

class VFile
{
public:
	VFile() = default;
	VFile( const std::filesystem::path &filename, const std::string &pathid, FileSystem *fileSystem );
	virtual ~VFile();

	bool is_open() const noexcept { return ( file != nullptr ); }

	void open( const std::filesystem::path &filename, const std::string &pathid, FileSystem *fileSystem );
	void close();

	std::size_t read( char *buffer, std::size_t count );
	std::size_t read( char *buffer, std::size_t size, std::size_t count );

	long int tell() { return ftell( file ); }

	bool seek( long int offset, int origin );
	bool eof() const;

	std::size_t file_size() const { return end - start; }

private:


	FILE *file = nullptr;
	std::size_t start = 0;
	std::size_t end = 0;

	bool iseof = false;
};

#endif // VFILE_HPP