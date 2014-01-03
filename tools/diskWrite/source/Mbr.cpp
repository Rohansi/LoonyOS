/*
 *	mbr.cpp
 *	Master Boot Record
 */

#include <fstream>
#include "mbr.hpp"

Mbr::Mbr(const std::string& filename) : exists(false) {
	std::ifstream f(filename.c_str(), std::ios::in | std::ios::binary);
	if (!f.is_open()) {
        throw std::exception("Failed to open disk");
	}

	f.seekg(446, std::ios::beg);

	for(int i = 0; i < 4; ++i) {
		PartitionEntry entry;
		f.read((char*)&entry, 16);

		if (entry.status == MBR_ENTRY_STATUS_ACTIVE)
			exists = true;

		partitions.push_back(entry);
	}

	f.close();
}

Mbr::~Mbr() {

}

const PartitionEntry* Mbr::GetPartitionEntry(size_t i) const {
	if (i > 4)
		return NULL;

	return &partitions[i];
}

const PartitionEntry* Mbr::GetActivePartition() const {
	for(int i = 0; i < 4; ++i)
		if (partitions[i].status == MBR_ENTRY_STATUS_ACTIVE)
			return &partitions[i];

	return NULL;
}

bool Mbr::Exists() const {
	return exists;
}
