#ifndef __SCRIPT_HPP
#define __SCRIPT_HPP

#include <iostream>
#include <string>
#include <fstream>

#include "Filesystem.hpp"
#include "Path.hpp"
#include "Mbr.hpp"

extern "C" {
    #include <lua.h>
    #include <lualib.h>
    #include <lauxlib.h>
}

class Script {
public:
    Script();
    ~Script();

    void Run(const std::string& filename, lua_Hook hook = NULL);

    void LoadFilesystem(const std::string& filename);
    void UnloadFilesystem();

    Filesystem* filesystem;

private:
    void Register(lua_State* L, const std::string& name, lua_CFunction func);

    int line;
};

int LoadDriver(lua_State* L);
int Mount(lua_State* L);
int Dismount(lua_State* L);
int ChDir(lua_State* L);
int RawWrite(lua_State* L);
int CopyIn(lua_State* L);
int CopyOut(lua_State* L);
int Delete(lua_State* L);
int Rename(lua_State* L);
int DirCreate(lua_State* L);
int DirDelete(lua_State* L);
int Format(lua_State* L);
int Exists(lua_State* L);

#endif
