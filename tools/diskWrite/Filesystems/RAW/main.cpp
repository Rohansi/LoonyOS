#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#define exp extern "C" __declspec(dllexport)

size_t fsize = 0;
std::fstream f;

std::string lastError;
exp const char* LastError() {
    return lastError.c_str();
}

exp bool Open(const char* fname, unsigned int a, unsigned int b) {
    f.open(fname, std::ios::in | std::ios::out | std::ios::binary);

    if (!f.is_open())
        return false;

    f.seekg(0, std::ios::end);
    fsize = (int)f.tellg();
    f.seekg(0);

    return true;
}

exp bool Close() {
    f.close();
    return true;
}

// expects a filename in the format of "offset.length"
exp bool Exists(const char* name) {
    std::istringstream ss(name);

    if (!ss.str().length())
        return false;

    size_t offset = 0, size = 0;

    ss >> offset;

    if (!ss.good() || ss.get() != '.')
        return false;

    ss >> size;

    if (ss.fail() || !size || !offset)
        return false;

    if (offset + size <= fsize)
        return true;

    return false;
}

exp bool DirOpenRoot() {
    return true; // raw is always in root
}

exp bool RawWrite(const char* fname, size_t offsetIn, size_t offset, size_t len) {
    if (!f.is_open() || offset + len > fsize)
        return false;

    std::ifstream in;
    int sz;

    in.open(fname, std::ios::in | std::ios::binary);

    if (!in.is_open())
        return false;

    in.seekg(0, std::ios::end);
    sz = (int)in.tellg();
    in.seekg(offsetIn);

    if ((sz - offsetIn) < len) {
        in.close();
        return false;
    }

    char* buff = new char[len];

    in.read(buff, len);
    f.seekp(offset);
    f.write(buff, len);

    delete[] buff;

    in.close();

    return true;
}

exp bool Format(const char*, const char*) {
    f.seekp(std::ios::beg);

    for (size_t i = 0; i < fsize; ++i)
        f.write("\0", 1);

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
