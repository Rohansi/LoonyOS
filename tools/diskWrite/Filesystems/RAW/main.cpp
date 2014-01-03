#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#define exp extern "C" __declspec(dllexport)

typedef struct _OffsetLength {
    size_t offset;
    size_t length;
} OffsetLength;

// expects a filename in the format of "offset.length"
bool ParseFileName(const char* name, OffsetLength* result) {
    std::istringstream ss(name);

    if (!ss.str().length())
        return false;

    size_t offset = 0, length = 0;

    ss >> offset;

    if (!ss.good() || ss.get() != '.')
        return false;

    ss >> length;

    if (ss.fail() || !ss.eof() || !length)
        return false;

    result->offset = offset;
    result->length = length;
    return true;
}

std::fstream disk;
size_t diskSize = 0;

std::string lastError;
exp const char* LastError() {
    return lastError.c_str();
}

void LastError(const std::string& scope, const std::string& msg) {
    lastError = "RAW - " + scope + ": " + msg;
}

exp bool Open(const char* name, unsigned int partitionOffset, unsigned int partitionLength) {
    disk.open(name, std::ios::in | std::ios::out | std::ios::binary);

    if (!disk.is_open())
        return false;

    disk.seekg(0, std::ios::end);
    diskSize = disk.tellg();
    disk.seekg(0);

    return true;
}

exp bool Close() {
    disk.close();
    return true;
}

exp bool RawWrite(const char* fileIn, size_t offsetIn, size_t offset, size_t len) {
    if ((offset + len) > diskSize) {
        LastError("RawWrite", "Destination out of range");
        return false;
    }

    std::ifstream in;
    size_t size;

    in.open(fileIn, std::ios::in | std::ios::binary);

    if (!in.is_open()) {
        LastError("RawWrite", "Failed to open input file");
        return false;
    }

    in.seekg(0, std::ios::end);
    size = in.tellg();
    in.seekg(offsetIn);

    if ((size - offsetIn) < len) {
        LastError("RawWrite", "Input file too small");
        in.close();
        return false;
    }

    char* buff = new char[len];

    in.read(buff, len);
    disk.seekp(offset);
    disk.write(buff, len);

    delete[] buff;
    in.close();

    return true;
}

exp bool DirOpenRoot() {
    return true; // raw is always in root
}

exp bool Exists(const char* name) {
    OffsetLength range;
    if (!ParseFileName(name, &range)) {
        LastError("Exists", "Invalid filename");
        return false;
    }

    if ((range.offset + range.length) > diskSize) {
        LastError("Exists", "Out of range");
        return false;
    }

    return ((range.offset + range.length) <= diskSize);
}

exp bool Format(const char* volumeName) {
    disk.seekp(std::ios::end);

    for (size_t i = 0; i < diskSize; ++i)
        disk.write("\0", 1);

    return true;
}

/*

exp bool CopyIn(const char* source, const char* dest) {
return false;
}

exp bool CopyOut(const char* source, const char* dest) {
return false;
}

// copy data?
exp bool Copy(const char* source, const char* dest) {
return false;
}

// move data?
exp bool Rename(const char* oldName, const char* newName) {
return false;
}

// null data?
exp bool Delete(const char* name) {
return false;
}

*/
