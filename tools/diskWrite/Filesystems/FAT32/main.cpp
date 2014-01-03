/*
*	main.cpp
*	FAT32 diskWrite Driver
*/

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <time.h>
#include "filename.hpp"
#include "common.hpp"

// Note - Support backup FAT?

Bpb* bpb;
FileSystemInfo* fsInfo;
Directory directory;
char *volumeId, *sectorBuffer;
size_t fsize = 0;
std::fstream f;

unsigned int partitionOffset, partitionLength;

// Prototype
void DirectoryWrite(Directory* dir);
exp bool DirOpenRoot();
exp bool DirOpen(const char* name);
exp bool DirUp();

void ReadSector(char* buffer, size_t sec) {
    sec += partitionOffset;
    if (sec > partitionLength)
        return;

    f.seekg(sec * 512, std::ios::beg);
    f.read(buffer, 512);
}

void ReadSectors(char* buffer, size_t sec, size_t count) {
    for (size_t i = 0; i < count; i++)
        ReadSector((char*)(buffer + (i * 512)), sec + i);
}

void WriteSector(char* buffer, size_t sec) {
    sec += partitionOffset;
    if (sec > partitionLength)
        return;

    if ((sec * 512) > fsize) {
        std::cout << "WOAH BRO THIS SECTOR IS WRONG: " << sec << std::endl;
        return;
    }

    f.seekp(sec * 512, std::ios::beg);
    f.write(buffer, 512);
}

void WriteSectors(char* buffer, size_t sec, size_t count) {
    for (size_t i = 0; i < count; i++)
        WriteSector((char*)(buffer + (i * 512)), sec + i);
}

inline bool ReadBPB() {
    bpb = new Bpb();
    ReadSector((char*)bpb, 0);

    // The 1000 years of dirty no good shameful disgusting shame hack
    bpb->hiddenSectors = 1;

    return true;
}

inline void WriteFSInfo() {
    /*std::cout << "Updating FS Info! sector = " << bpb->clusterFSInfo << std::endl;
    std::cout << "nextFreeSector = " << fsInfo->nextFreeCluster << std::endl;
    std::cout << "freeSectors = " << fsInfo->freeClusterCount << std::endl;*/

    WriteSector((char*)fsInfo, bpb->clusterFSInfo);
}

// Expects DOS8.3 format
DirectoryEntry* FindEntry(const char* filename, const Directory* dir) {
    DirectoryEntry* entry;
    char* buffer;

    for (size_t k = 0; k < dir->count; k++) {
        buffer = dir->sectors[k]->buffer;

        for (size_t i = 0; i < bpb->bytesPerSector; i += sizeof(DirectoryEntry)) {
            entry = (DirectoryEntry*)(buffer + i);
            if (!entry)
                return 0;

            if (strncmp(filename, entry->name, 11) != 0)
                continue;

            return entry;
        }
    }

    return 0;
}

DirectoryEntry* FindEntryFree(const Directory* dir) {
    DirectoryEntry* entry;
    char* buffer;

    for (size_t k = 0; k < dir->count; k++) {
        buffer = dir->sectors[k]->buffer;

        for (size_t i = 0; i < bpb->bytesPerSector; i += sizeof(DirectoryEntry)) {
            entry = (DirectoryEntry*)(buffer + i);
            if (!entry)
                return 0;

            if (!entry->name[0] || entry->name[0] == ATTRIB_DELETED)
                return entry;
        }
    }

    return 0;
}

DirectoryEntry* FindEntryAttribute(const unsigned char attrib, const Directory* dir) {
    DirectoryEntry* entry;
    char* buffer;

    for (size_t k = 0; k < dir->count; k++) {
        buffer = dir->sectors[k]->buffer;

        for (size_t i = 0; i < bpb->bytesPerSector; i += sizeof(DirectoryEntry)) {
            entry = (DirectoryEntry*)(buffer + i);
            if (!entry)
                return 0;

            if (entry->attrib != attrib)
                continue;

            return entry;
        }
    }

    return 0;
}

uint32_t FindFreeCluster() {
    char* buffer = new char[bpb->bytesPerSector];
    uint32_t sector, i = fsInfo->nextFreeCluster;

    sector = GetFATSector(fsInfo->nextFreeCluster);
    ReadSector(buffer, sector);

    for (; i < bpb->sectorsPerFATLarge * bpb->bytesPerSector; i++) {
        // Read next FAT sector if required
        if (sector != GetFATSector(i)) {
            sector = GetFATSector(i);
            ReadSector(buffer, sector);
        }

        // Check if this entry is free
        if (*(uint32_t*)(buffer + GetFATOffset(i)) == FAT_FREE)
            break;
    }

    if (i >= ((bpb->sectorsPerFATLarge * bpb->bytesPerSector) / 4)) {
        std::cout << "No free clusters!" << std::endl;
        return FAT_EOF;
    }

    // Update fsInfo, since we found a new free cluster
    fsInfo->nextFreeCluster = i;
    fsInfo->freeClusterCount--;

    delete[] buffer;

    //std::cout << "FAT i = " << i << ", spFATL = " << ((bpb->sectorsPerFATLarge * bpb->bytesPerSector) / 4) << std::endl;

    return i;
}

/*
*	entry->cluster  = last cluster (current EOF)
*	entry->sector   = current FAT sector to read/write
*	entry->buffer   = FAT buffer
*	cluster			= new EOF cluster
*/
void UpdateFAT(FatEntry* entry, uint32_t cluster) {
    // Update old EOF to point to new cluster
    *(uint32_t*)(entry->buffer + GetFATOffset(entry->cluster)) = cluster;

    // Check if we need to load a new FAT sector
    if (entry->sector != GetFATSector(cluster)) {
        WriteSector(entry->buffer, entry->sector);
        entry->sector = GetFATSector(cluster);
        ReadSector(entry->buffer, entry->sector);
    }

    // Set new cluster to EOF
    *(uint32_t*)(entry->buffer + GetFATOffset(cluster)) = FAT_EOF;
    WriteSector(entry->buffer, entry->sector);

    // Update FS Info here?
}

/*
*	entry = empty allocated DirectoryEntry
*	Expects that directoryBuffer, directorySectors, directorySector are all valid
*	entry will be moved to a valid place in memory
*	DOES NOT WRITE TO DISK, JUST MEMORY
*/
DirectoryEntry* MakeEntry(const char* filename, const uint8_t attrib, const uint32_t size, const Directory* dir) {
    // Get the time
    time_t timestamp;
    struct tm* localTime;
    time(&timestamp);
    localTime = localtime(&timestamp);

    // Find a free place for this entry
    DirectoryEntry* entry = FindEntryFree(dir);
    if (!entry) {
        std::cout << "No free entries!" << std::endl;
        return 0;
    }

    // Find a free cluster
    uint32_t tmpCluster = FindFreeCluster();
    if (tmpCluster == FAT_EOF)
        return 0;

    // Populate the entry
    memcpy(entry->name, filename, DOS83_MAX_LEN);
    entry->attrib = attrib;							// Attributes
    entry->creationTimeLo = 0;						// Creation time (milliseconds)
    entry->creationTimeHi = CreateTime(localTime);	// Creation time (hh:mm:ss)
    entry->creationDate = CreateDate(localTime);	// Creation date (yy:mm:dd)
    entry->lastModTime = entry->creationTimeHi;		// Modification time
    entry->lastModDate = entry->creationDate;		// Modification date
    entry->accessDate = entry->creationDate;		// Creation date

    entry->firstClusterHi = HIWORD(tmpCluster);		// First cluster
    entry->firstClusterLo = LOWORD(tmpCluster);		// First cluster

    entry->size = size;								// Size

    return entry;
}

// Updates everything except for starting cluster and name
// ONLY AFFECTS ENTRY, NOT FAT
void UpdateEntry(DirectoryEntry* entry, const uint8_t attrib, const uint32_t size) {
    // Get the time
    time_t timestamp;
    struct tm* localTime;
    time(&timestamp);
    localTime = localtime(&timestamp);

    // Update the entry
    entry->attrib = (attrib) ? attrib : entry->attrib;
    entry->size = (size) ? size : entry->size;
    entry->lastModDate = CreateDate(localTime);
    entry->lastModTime = CreateTime(localTime);
    entry->accessDate = CreateDate(localTime);

    // Write to disk
    DirectoryWrite(&directory);
    //WriteSector(directoryBuffer + ((((char*)entry - directoryBuffer) / 512) * 512),directorySector + (((char*)entry - directoryBuffer) / 512));
}

bool ReadCluster(FatEntry* entry, char* buffer) {
    // Load FAT sector if needed - Update FatEntry sector
    if (entry->sector != GetFATSector(entry->cluster)) {
        entry->sector = GetFATSector(entry->cluster);
        ReadSector(entry->buffer, entry->sector);
    }

    // Check if we're done
    if (*(uint32_t*)(entry->buffer + GetFATOffset(entry->cluster)) >= FAT_EOF) {
        ReadSector(buffer, ClusterToSector(entry->cluster));
        return false;
    }

    // Read sector 
    ReadSector(buffer, ClusterToSector(entry->cluster));

    // Update cluster
    entry->cluster = *(uint32_t*)(entry->buffer + GetFATOffset(entry->cluster));

    return true;
}

bool WriteCluster(FatEntry* fatEntry, char* buffer) {
    // Load FAT sector if needed - Update FatEntry sector
    if (fatEntry->sector != GetFATSector(fatEntry->cluster)) {
        fatEntry->sector = GetFATSector(fatEntry->cluster);
        ReadSector(fatEntry->buffer, fatEntry->sector);
    }

    // Write that sector
    WriteSector(buffer, ClusterToSector(fatEntry->cluster));

    // Find a new cluster
    uint32_t tmpCluster = FindFreeCluster();

    UpdateFAT(fatEntry, tmpCluster);
    fatEntry->cluster = tmpCluster;

    fsInfo->nextFreeCluster = tmpCluster;
    fsInfo->freeClusterCount--;

    return true;
}

bool DeleteCluster(FatEntry* entry) {
    // Load FAT sector if needed - Update FatEntry sector
    if (entry->sector != GetFATSector(entry->cluster)) {
        WriteSector(entry->buffer, entry->sector);
        entry->sector = GetFATSector(entry->cluster);
        ReadSector(entry->buffer, entry->sector);
    }

    // Find a new cluster
    uint32_t tmpCluster = *(uint32_t*)(entry->buffer + GetFATOffset(entry->cluster));

    *(uint32_t*)(entry->buffer + GetFATOffset(entry->cluster)) = FAT_FREE;
    entry->cluster = tmpCluster;

    WriteSector(entry->buffer, entry->sector);

    // Update FS Info
    if (tmpCluster < fsInfo->nextFreeCluster)
        fsInfo->nextFreeCluster = tmpCluster - 1;

    fsInfo->freeClusterCount++;

    return (tmpCluster < FAT_EOF);
}

void DirectoryRead(Directory* dir) {
    char* buffer = new char[bpb->bytesPerSector];

    FatEntry fatEntry;
    fatEntry.buffer = new char[bpb->bytesPerSector];
    fatEntry.cluster = dir->cluster;
    fatEntry.sector = GetFATSector(fatEntry.cluster);
    ReadSector(fatEntry.buffer, fatEntry.sector);

    Sector* sector;
    while (ReadCluster(&fatEntry, buffer)) {
        // Allocate a new sector
        sector = new Sector;
        sector->buffer = new char[512];
        sector->sector = ClusterToSector(fatEntry.cluster);
        memcpy(sector->buffer, buffer, 512);

        // Add to list
        directory.sectors[directory.count] = sector;
        directory.count++;
    }

    // Allocate a new sector
    sector = new Sector;
    sector->buffer = new char[512];
    sector->sector = ClusterToSector(fatEntry.cluster);
    memcpy(sector->buffer, buffer, 512);

    // Add to list
    directory.sectors[directory.count] = sector;
    directory.count++;

    delete[] buffer;
    delete[] fatEntry.buffer;
}

void DirectoryWrite(Directory* dir) {
    for (uint32_t i = 0; i < dir->count; i++)
        WriteSector(dir->sectors[i]->buffer, dir->sectors[i]->sector);
}

void DirectoryClear(Directory* dir) {
    for (uint32_t i = 0; i < dir->count; i++) {
        delete[] dir->sectors[i]->buffer;
        delete dir->sectors[i];
    }

    dir->count = dir->cluster = 0;
}

std::string lastError;
exp const char* LastError() {
    return lastError.c_str();
}

void LastError(const std::string& scope, const std::string& msg) {
    lastError = "FAT32 - " + scope + ": " + msg;
}

exp bool Open(const char* fname, unsigned int pO, unsigned int pLen) {
    partitionOffset = pO;
    partitionLength = pLen;

    // Open the image
    f.open(fname, std::ios::in | std::ios::out | std::ios::binary);

    if (!f.is_open()) {
        LastError("Open", "Failed to open disk");
        return false;
    }

    f.seekg(0, std::ios::end);
    fsize = (size_t)f.tellg();
    f.seekg(0);

    // Allocate a buffer
    sectorBuffer = new char[SECTOR_SIZE];

    // Read the BPB
    if (!ReadBPB()) {
        LastError("Open", "Failed to read the BPB");
        return false;
    }

    // Read filesystem info
    fsInfo = new FileSystemInfo;
    ReadSector((char*)fsInfo, bpb->clusterFSInfo);

    // Load the root directory
    if (!DirOpenRoot()) {
        LastError("Open", "Failed to load the root directory");
        return false;
    }

    volumeId = new char[DOS83_MAX_LEN + 1];
    ReadSector(sectorBuffer, ClusterToSector(bpb->clusterRoot));
    DirectoryEntry* entry = FindEntryAttribute(ATTRIB_VOLUME_ID, &directory);
    if (entry) {
        memcpy(volumeId, entry->name, DOS83_MAX_LEN);
        volumeId[11] = 0;
    }

    return true;
}

exp bool Close() {
    // Update FS info before unmounting the disk
    WriteFSInfo();
    f.close();

    return true;
}

exp bool Exists(const char* name) {
    return (FindEntry(name, &directory) != 0);
}

exp bool DirOpenRoot() {
    directory.cluster = bpb->clusterRoot;
    directory.count = 0;

    DirectoryRead(&directory);

    return true;
}

exp bool DirOpen(const char* name) {
    char* dosName = new char[12];
    ToDos83Name(name, dosName);

    // Open the entry
    DirectoryEntry* entry = FindEntry(dosName, &directory);
    if (!entry) {
        delete[] dosName;
        LastError("DirOpen", "Directory doesn't exist");
        return false;
    }

    // Check that it is in fact a directory
    if (!(entry->attrib & ATTRIB_DIRECTORY)) {
        delete[] dosName;
        LastError("DirOpen", "Not a directory");
        return false;
    }

    // Check if it's refering to root
    if (!GetClusterFromEntry(entry))
        return DirOpenRoot();

    uint32_t tmpCluster = GetClusterFromEntry(entry);
    DirectoryClear(&directory);

    directory.cluster = tmpCluster;
    directory.count = 0;

    DirectoryRead(&directory);

    // Find its sector
    //directory.sector = ClusterToSector(GetClusterFromEntry(entry));
    //directory.count = 2;

    //// Clean up old directory buffer and load new directory
    //if (directoryBuffer)
    //	delete[] directoryBuffer;

    //directoryBuffer = new char[directorySectors * bpb->bytesPerSector];

    //ReadSectors(directoryBuffer,directorySector,directorySectors);

    delete[] dosName;

    return true;
}

exp bool DirUp() {
    return DirOpen("..         ");
}

exp bool Copy(const char* source, const char* dest) {
    return false;
}

exp bool RawWrite(const char* fname, size_t offsetIn, size_t offset, size_t len) {
    std::ifstream in(fname, std::ios::in | std::ios::binary);
    if (!in.is_open())
        return false;

    // Seek to proper offsets
    in.seekg(offsetIn, std::ios::beg);
    f.seekp(offset + partitionOffset * 512, std::ios::beg);

    // Read data
    char* buffer = new char[len];
    in.read(buffer, len);
    f.write(buffer, len);

    delete[] buffer;
    in.close();

    return true;
}

exp bool Rename(const char* old, const char* newname) {
    // Convert the filename into DOS8.3 format
    char* dosName = new char[12];
    char* dosNameNew = new char[12];
    ToDos83Name(old, dosName);
    ToDos83Name(newname, dosNameNew);

    // Find the entry
    DirectoryEntry* entry = FindEntry(dosName, &directory);
    if (!entry) {
        LastError("Rename", "Failed to find file");
        return false;
    }

    memcpy(entry->name, dosNameNew, DOS83_MAX_LEN);

    DirectoryWrite(&directory);
    //WriteSector(directoryBuffer + ((((char*)entry - directoryBuffer) / 512) * 512),directorySector + (((char*)entry - directoryBuffer) / 512));

    delete[] dosName;
    delete[] dosNameNew;

    return true;
}

exp bool Delete(const char* filename) {
    // Convert the filename into DOS8.3 format
    char* dosName = new char[12];
    ToDos83Name(filename, dosName);

    // Find entry
    DirectoryEntry* entry = FindEntry(dosName, &directory);
    if (!entry) {
        LastError("Delete", "Failed to find file");
        return false;
    }

    if (!(entry->attrib & ATTRIB_ARCHIVE) || (entry->attrib & ATTRIB_DIRECTORY)) {
        LastError("Delete", "This is not a file");
        return false;
    }

    entry->name[0] = (char)ATTRIB_DELETED;

    DirectoryWrite(&directory);
    //WriteSector(directoryBuffer + ((((char*)entry - directoryBuffer) / 512) * 512),directorySector + (((char*)entry - directoryBuffer) / 512));

    // Unlink all clusters
    FatEntry fatEntry;
    fatEntry.buffer = new char[bpb->bytesPerSector];
    fatEntry.cluster = GetClusterFromEntry(entry);
    fatEntry.sector = GetFATSector(fatEntry.cluster);
    ReadSector(fatEntry.buffer, fatEntry.sector);

    while (DeleteCluster(&fatEntry))
        Sleep(1);

    WriteSector(fatEntry.buffer, fatEntry.sector);

    delete[] fatEntry.buffer;

    return false;
}

exp bool DirCreate(const char* dirName) {
    // Convert the filename into DOS8.3 format
    char* dosName = new char[12];
    ToDos83Name(dirName, dosName);

    DirectoryEntry* entry = MakeEntry(dosName, ATTRIB_DIRECTORY, 0, &directory);
    if (!entry) {
        LastError("DirCreate", "Failed to create a directory");
        delete[] dosName;
        return false;
    }

    // Write FAT entry
    FatEntry fatEntry;
    fatEntry.buffer = new char[bpb->bytesPerSector];
    fatEntry.cluster = GetClusterFromEntry(entry);
    fatEntry.sector = GetFATSector(fatEntry.cluster);

    UpdateFAT(&fatEntry, GetClusterFromEntry(entry));

    // Write entry to disk
    DirectoryWrite(&directory);
    //WriteSector(directoryBuffer + ((((char*)entry - directoryBuffer) / 512) * 512),directorySector + (((char*)entry - directoryBuffer) / 512));

    delete[] dosName;
    delete[] fatEntry.buffer;

    return true;
}

exp bool DirDelete(const char* path) {

    return false;
}

exp bool CopyIn(const char* source, const char* dest) {
    DirectoryEntry* entry = 0;
    char* buffer = new char[bpb->bytesPerSector];

    // Open file we're copying in
    std::ifstream inHandle(source, std::ios::in | std::ios::binary | std::ios::ate);
    if (!inHandle.is_open()) {
        LastError("CopyIn", "Failed to open source file");
        return false;
    }

    // Get size of source file
    size_t inLength = (size_t)inHandle.tellg();
    inHandle.seekg(std::ios::beg);

    // Convert the filename into DOS8.3 format
    char* dosName = new char[12];
    ToDos83Name(dest, dosName);

    // Check if entry exists, if so, overwrite it
    entry = FindEntry(dosName, &directory);
    if (entry) {
        // Delete file
        Delete(dest);

        // Undelete entry
        entry->name[0] = dosName[0];
    }

    // Create the directory entry
    if (!entry) {
        // Didn't delete file, make it
        entry = MakeEntry(dosName, ATTRIB_ARCHIVE, inLength, &directory);
        if (!entry) {
            LastError("CopyIn", "Failed to create directory entry");
            delete[] dosName;
            delete[] buffer;
            return false;
        }
    }
    else {
        // Deleted file, find it
        entry = FindEntry(dosName, &directory);
        if (!entry) {
            LastError("CopyIn", "Somehow a file that was just undeleted can't be found...");
            delete[] dosName;
            delete[] buffer;
            return false;
        }

        UpdateEntry(entry, 0, inLength);
    }

    // Write entry to disk
    DirectoryWrite(&directory);

    // Update FAT and then load each cluster and write it while updating the FAT
    FatEntry fatEntry;
    fatEntry.buffer = new char[bpb->bytesPerSector];
    fatEntry.cluster = GetClusterFromEntry(entry);
    fatEntry.sector = GetFATSector(fatEntry.cluster);
    ReadSector(fatEntry.buffer, fatEntry.sector);

    UpdateFAT(&fatEntry, FindFreeCluster());

    // Write to disk
    for (size_t i = 0; i < inLength; i += bpb->bytesPerSector) {
        memset(buffer, 0, bpb->bytesPerSector);
        if (!(i % bpb->bytesPerSector))
            inHandle.read(buffer, bpb->bytesPerSector);
        else
            inHandle.read(buffer, i % bpb->bytesPerSector);

        WriteCluster(&fatEntry, buffer);
    }

    delete[] dosName;
    delete[] buffer;
    delete[] fatEntry.buffer;

    inHandle.close();

    return true;
}

exp bool CopyOut(const char* source, const char* dest) {
    // Convert the filename into DOS8.3 format
    char* dosName = new char[12];
    ToDos83Name(source, dosName);

    // Find the file on the disk
    DirectoryEntry* entry = FindEntry(dosName, &directory);
    if (!entry) {
        LastError("CopyOut", "File doesn't exist");
        return false;
    }

    if (!(entry->attrib & ATTRIB_ARCHIVE)) {
        LastError("CopyOut", "This is not a file");
        return false;
    }

    // Update the entry
    UpdateEntry(entry, 0, 0);

    std::ofstream outHandle(dest, std::ios::out | std::ios::binary);
    if (!outHandle.is_open())
        return false;

    // Set up a FatEntry
    FatEntry fatEntry;
    fatEntry.cluster = GetClusterFromEntry(entry);
    fatEntry.sector = GetFATSector(fatEntry.cluster);
    fatEntry.buffer = new char[bpb->bytesPerSector];
    ReadSector(fatEntry.buffer, fatEntry.sector);

    char* sectorData = new char[bpb->bytesPerSector];

    // Read Data
    while (ReadCluster(&fatEntry, sectorData))
        outHandle.write(sectorData, 512);

    if (!(entry->size % bpb->bytesPerSector))
        outHandle.write(sectorData, 512);
    else
        outHandle.write(sectorData, entry->size % bpb->bytesPerSector);

    delete[] fatEntry.buffer;
    delete[] sectorData;
    delete[] dosName;

    outHandle.close();

    return true;
}

/*exp bool Format(const char* volumeName) {
    return false;
}*/
