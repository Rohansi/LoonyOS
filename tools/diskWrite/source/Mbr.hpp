/*
 *	Mbr.hpp
 *	Master Boot Record
 */

#ifndef __MBR_HPP
#define __MBR_HPP

#include <iostream>
#include <vector>
#include <string>

#define MBR_ENTRY_STATUS_ACTIVE			0x80

typedef struct _PartitionEntry {
    unsigned char	status;
    unsigned char	chrFirst[3];
    unsigned char	type;
    unsigned char	chsLast[3];
    unsigned int	lbaFirst;
    unsigned int	length;
} PartitionEntry;

class Mbr {
private:
    std::vector<PartitionEntry> partitions;
    bool exists;

public:
    Mbr(const std::string& filename);
    ~Mbr();

    const PartitionEntry* GetPartitionEntry(size_t i) const;
    const PartitionEntry* GetActivePartition() const;

    bool Exists() const;
};

#endif
