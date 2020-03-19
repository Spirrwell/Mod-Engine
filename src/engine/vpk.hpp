#ifndef VPK_HPP
#define VPK_HPP

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "mount.hpp"

constexpr uint32_t VPK_SIGNATURE = 0x55aa1234;

struct VPKHeader
{
	// Version 1 Info

	uint32_t Signature = 0;
	uint32_t Version = 0;

	// The size, in bytes, of the directory tree
	uint32_t TreeSize = 0;

	// Version 2 Info

	// How many bytes of file content are stored in this VPK file (0 in CSGO)
	uint32_t FileDataSectionSize = 0;

	// The size, in bytes, of the section containing MD5 checksums for external archive content
	uint32_t ArchiveMD5SectionSize = 0;

	// The size, in bytes, of the section containing MD5 checksums for content in this file (should always be 48)
	uint32_t OtherMD5SectionSize = 0;

	// The size, in bytes, of the section containing the public key and signature. This is either 0 (CSGO & The Ship) or 296 (HL2, HL2:DM, HL2:EP1, HL2:EP2, HL2:LC, TF2, DOD:S & CS:S)
	uint32_t SignatureSectionSize = 0;

	size_t GetVersionSize() const { if ( Version == 1 ) return 12; if ( Version == 2 ) return 28; return 0; }
};

struct VPKDirectoryEntry
{
	uint32_t CRC; // A 32bit CRC of the file's data.
	uint16_t PreloadBytes; // The number of bytes contained in the index file.

	// A zero based index of the archive this file's data is contained in.
	// If 0x7fff, the data follows the directory.
	uint16_t ArchiveIndex;

	// If ArchiveIndex is 0x7fff, the offset of the file data relative to the end of the directory (see the header for more details).
	// Otherwise, the offset of the data from the start of the specified archive.
	uint32_t EntryOffset;

	// If zero, the entire file is stored in the preload data.
	// Otherwise, the number of bytes stored starting at EntryOffset.
	uint32_t EntryLength;

	uint16_t Terminator = 0xffff; // This should always be 0xffff
};

struct VPK_ArchiveMD5SectionEntry
{
	uint32_t ArchiveIndex;
	uint32_t StartingOffset; // where to start reading bytes
	uint32_t Count; // how many bytes to check
	char MD5Checksum[ 16 ]; // expected checksum
};

struct VPK_OtherMD5Section
{
	char TreeChecksum[ 16 ];
	char ArchiveMD5SectionChecksum[ 16 ];
	char Unknown[ 16 ];
};

struct VPKFileEntry
{
	VPKDirectoryEntry directoryEntry;
	std::filesystem::path filename;
};

class VPK : public Mount
{
public:
	VPK( const std::filesystem::path &directoryPath );

	bool IsValid() const override { return ( directoryFile.get() != nullptr ); }

	CaseSensitivity GetCaseSensitivity() const { return CaseSensitivity::Lower; }

	MountFileHandle OpenFile( std::size_t index ) override;
	FileSystem::MountFindResult FindFile( const std::filesystem::path &relpath ) const override;

	bool ReadToBuffer( std::vector< char > &buffer, std::size_t index ) override;

private:

	struct ArchiveInfo
	{
		unique_ptr< std::ifstream > file; // We keep the archive files open to prevent deletion
		std::filesystem::path archivePath;
	};

	bool isArchive( uint16_t archiveIndex ) const noexcept { return ( archiveIndex != 0x7fff ); }

	VPKHeader vpkHeader;

	unique_ptr< std::ifstream > directoryFile; // We keep the directory file open to prevent deletion
	const std::filesystem::path directoryPath;

	std::vector< VPKFileEntry > fileEntries;
	std::vector< ArchiveInfo > archiveInfos;
};

#endif // VPK_HPP