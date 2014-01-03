#pragma once

#include <string>
#include <windows.h>

class Filesystem {
public:
    Filesystem(const std::string& dll);
    ~Filesystem();

    bool        IsLoaded();
    const char* LastError();
    void        Open(const char* name, unsigned int partitionOffset, unsigned int partitionLength);
    void        Close();
    bool        Exists(const char* name);
    void        DirOpenRoot();
    void        DirOpen(const char* name);
    void        DirUp();
    void        Copy(const char* source, const char* dest);
    void        Rename(const char* oldName, const char* newName);
    void        Delete(const char* name);
    void        DirCreate(const char* name);
    void        DirDelete(const char* name);
    void        RawWrite(const char* fname, size_t offsetIn, size_t offset, size_t length);
    void        CopyIn(const char* source, const char* dest);
    void        CopyOut(const char* source, const char* dest);
    void        Format(const char* volumeName);

private:
    HMODULE hm;

    const char* (*_LastError)           ();
    bool        (*_Open)                (const char*, unsigned int, unsigned int);
    bool        (*_Close)               ();
    bool        (*_Exists)              (const char*, bool*);
    bool        (*_DirOpenRoot)         ();
    bool        (*_DirOpen)             (const char*);
    bool        (*_DirUp)               ();
    bool        (*_Copy)                (const char*, const char*);
    bool        (*_Rename)              (const char*, const char*);
    bool        (*_Delete)              (const char*);
    bool        (*_DirCreate)           (const char*);
    bool        (*_DirDelete)           (const char*);
    bool        (*_RawWrite)            (const char*, size_t, size_t, size_t);
    bool        (*_CopyIn)              (const char*, const char*);
    bool        (*_CopyOut)             (const char*, const char*);
    bool        (*_Format)              (const char*);
};
