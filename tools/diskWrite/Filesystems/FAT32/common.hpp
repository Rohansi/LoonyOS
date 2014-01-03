/*
 *	common.hpp
 *	Common constants & macros
 */

#ifndef __COMMON_HPP
#define __COMMON_HPP

#include <stdint.h>
#include <windows.h>

#define exp extern "C" __declspec(dllexport)

// Constants
#define ATTRIB_READ_ONLY			0x01
#define ATTRIB_HIDDEN				0x02
#define ATTRIB_SYSTEM				0x04
#define ATTRIB_VOLUME_ID			0x08
#define ATTRIB_DIRECTORY			0x10
#define ATTRIB_ARCHIVE				0x20
#define ATTRIB_LONG_NAME			(ATTRIB_READ_ONLY | ATTRIB_HIDDEN | ATTRIB_SYSTEM | ATTRIB_VOLUME_ID)
#define ATTRIB_DELETED				0xE5

#define FAT_BAD						0x0FFFFFF7
#define FAT_EOF						0x0FFFFFF8
#define FAT_FREE					0x00000000

#define SECTOR_SIZE					512
#define SECTOR_DATA					(bpb->reservedSectors + (bpb->numberOfFATs * bpb->sectorsPerFATLarge))

#define DOS83_MAX_LEN				11

#define SECTOR_BPB					0

// Macros
#define ClusterToSector(n)			((((n) - 2) * bpb->sectorsPerCluster) + SECTOR_DATA)
#define GetClusterFromEntry(e)		(((e)->firstClusterHi << 8) | (e)->firstClusterLo)
#define GetFATSector(n)				(bpb->reservedSectors + (((n) * 4) / bpb->bytesPerSector))
#define GetFATOffset(n)				(((n) * 4) % bpb->bytesPerSector)

#define CreateTime(t)				(((t)->tm_hour << 11) + ((t)->tm_min << 5) + ((t)->tm_sec >> 1))
#define CreateDate(t)				((((t)->tm_year - 2000) << 9) + ((t)->tm_mon << 5) + (t)->tm_mday)
#define FileTimeToUNIXTime(t)		(t)

// Typedefs
#pragma pack(push,1)

// BPB
typedef struct _Bpb {
	uint8_t		jmp[3];				// Jump code
	char		oem[8];				// OEM Name
	uint16_t	bytesPerSector;		// Bytes Per Sector
	uint8_t		sectorsPerCluster;	// Sectors per cluster
	uint16_t	reservedSectors;	// Reserved sectors
	uint8_t		numberOfFATs;		// Number of FATs
	uint16_t	directoryEntries;	// Number of root directory entries
	uint16_t	totalSectors;		// Should be 0
	uint8_t		media;				// Should be 0xF8
	uint16_t	sectorsPerFAT;		// Should be 0
	uint16_t	sectorsPerTrack;	// Sectors per track
	uint16_t	heads;				// Number of heads
	uint32_t	hiddenSectors;		// Number of hidden sectors
	uint32_t	totalSectorsLarge;	// Set if more than 65535 sectors

	// Extended BPB (FAT32 specific)
	uint32_t	sectorsPerFATLarge;	// Number of sectors per FAT
	uint16_t	flags;				// Flags
	uint16_t	version;			// FAT32 Version
	uint32_t	clusterRoot;		// Offset of root directory
	uint16_t	clusterFSInfo;		// Offset of FS Info
	uint16_t	clusterBackupBS;	// Offset to backup bootsector
	uint8_t		reserved[12];		// Should be 0
	uint8_t		driverNumber;		// Should be 0x80
	uint8_t		reserved2;			// Reserved for WinNT
	uint8_t		signature;			// Should be 0x28 or 0x29
	uint32_t	serial;				// Serial ID
	char		label[11];			// Should be padded with spaces
	char		fsys[8];			// Filesystem

	// Non-BPB specific
	uint8_t		code[420];			// Boot code
	uint16_t	bootSig;			// Boot signature, must be 0xAA55
} Bpb;

// Filesystem info
typedef struct _FileSystemInfo {
	uint32_t	LeadSig;			// Lead signature
	uint8_t		reserved[480];		// Reserved
	uint32_t	StrucSig;			// Structure signature
	uint32_t	freeClusterCount;	// Number of free clustures on the disk
	uint32_t	nextFreeCluster;	// Next free cluster
	uint8_t		reserved2[12];		// Reserved
} FileSystemInfo;

// Directory Entry
typedef struct _DirectoryEntry {
	char		name[11];			// 8.3 format
	uint8_t		attrib;				// Attributes
	uint8_t		reserved;			// Reserved for WinNT
	uint8_t		creationTimeLo;		// Creation time
	uint16_t	creationTimeHi;		// Creation time
	uint16_t	creationDate;		// Creation date
	uint16_t	accessDate;			// Access date
	uint16_t	firstClusterHi;		// First cluster
	uint16_t	lastModTime;		// Last modification time
	uint16_t	lastModDate;		// Last modification date
	uint16_t	firstClusterLo;		// First cluster
	uint32_t	size;				// File size
} DirectoryEntry;

// Long Filenames
typedef struct _LongFilename {
	uint8_t		order;				// The position of this entry in the name
	uint16_t	nameLo[5];			// First 5 characters
	uint8_t		attrib;				// Always 0x0F
	uint8_t		type;				// LE type
	uint8_t		checksum;			// Checksum
	uint16_t	nameMid[6];			// Next 6 characters
	uint16_t	reserved;			// Always 0
	uint16_t	nameHi[2];			// Last 2 characters
} LongFilename;

typedef struct _Sector {
	uint32_t	sector;
	char*		buffer;
} Sector;

typedef struct _Directory {
	uint32_t	cluster, count;
	Sector*		sectors[256];
} Directory;

typedef struct _FatEntry {
	uint32_t	cluster, sector;	// Current cluster and current FAT sector
	char*		buffer;				// Current FAT sector
} FatEntry;

#pragma pack(pop)

#endif
