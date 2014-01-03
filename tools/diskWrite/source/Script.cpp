#include "Script.hpp"

Script::Script() {
	filesystem = NULL;
}

Script::~Script() {
    if (filesystem) {
        delete filesystem;
        filesystem = NULL;
	}
}

void Script::Run(const std::string& filename, lua_Hook hook) {
	lua_State* L = luaL_newstate();

    if (L == NULL) {
        throw std::exception("Failed to create Lua state");
    }

	luaL_openlibs(L);

	Register(L, "LoadDriver", LoadDriver);
    Register(L, "Mount", Mount);
    Register(L, "Dismount", Dismount);
    Register(L, "ChDir", ChDir);
    Register(L, "RawWrite", RawWrite);
    Register(L, "CopyIn", CopyIn);
    Register(L, "CopyOut", CopyOut);
    Register(L, "Delete", Delete);
    Register(L, "Rename", Rename);
    Register(L, "DirCreate", DirCreate);
    Register(L, "DirDelete", DirDelete);
    Register(L, "Format", Format);
    Register(L, "Exists", Exists);

    if (hook)
        lua_sethook(L, hook, LUA_MASKLINE, 0);

	if (luaL_dofile(L, filename.c_str())) {
        std::exception e(lua_tostring(L, 1));
		lua_close(L);
        throw e;
	}

	lua_close(L);
}

void Script::Register(lua_State* L, const std::string& name, lua_CFunction func) {
    lua_pushlightuserdata(L, this);
    lua_pushcclosure(L, func, 1);
    lua_setglobal(L, name.c_str());
}

void Script::LoadFilesystem(const std::string& filename) {
    UnloadFilesystem();

    filesystem = new Filesystem(filename);
    if (!filesystem->IsLoaded()) {
        throw std::exception("Failed to load filesystem driver");
    }
}

void Script::UnloadFilesystem() {
    if (filesystem) {
        delete filesystem;
        filesystem = NULL;
    }
}

Script* GetScript(lua_State* L) {
    if (!lua_islightuserdata(L, lua_upvalueindex(1)))
        throw std::exception("broken the luas");

    return (Script*)lua_touserdata(L, lua_upvalueindex(1));
}



// LoadDriver("RAW.dll")
int LoadDriver(lua_State* L) {
    Script* script = GetScript(L);

	if (lua_gettop(L) != 1 || !lua_isstring(L, 1)) {
        throw std::exception("LoadDriver: invalid parameters");
	}

    script->LoadFilesystem(lua_tostring(L, 1));
	return 0;
}

// Mount("RAW.img")
int Mount(lua_State* L) {
    Script* script = GetScript(L);

	if (lua_gettop(L) != 1 || !lua_isstring(L, 1)) {
		throw std::exception("Mount: invalid parameters");
	}

	unsigned int partitionOffset, partitionLength;
	Mbr mbr(lua_tostring(L, 1));

	if (mbr.Exists()) {
		const PartitionEntry* partitionEntry = mbr.GetActivePartition();

		if (partitionEntry == NULL) {
			throw std::exception("Mount: MBR is active but no partition is active!");
		}

		partitionOffset = partitionEntry->lbaFirst;
		partitionLength = partitionEntry->length;
	} else {
		partitionOffset = 0;
		
		std::ifstream f(lua_tostring(L, 1), std::ios::in | std::ios::binary);
	
		if (!f.is_open()) {
            throw std::exception("Mount: Failed to open disk");
		}

		f.seekg(0, std::ios::end);
		partitionLength = (size_t)f.tellg() / 512;
		f.close();
	}

	script->filesystem->Open(lua_tostring(L, 1), partitionOffset, partitionLength);
	return 0;
}

// Dismount()
int Dismount(lua_State* L) {
    Script* script = GetScript(L);

	if (lua_gettop(L) != 0) {
        throw std::exception("Dismount: invalid parameters");
	}

	script->filesystem->Close();
	return 0;
}

// ChDir("/system/")
int ChDir(lua_State* L) {
    Script* script = GetScript(L);
    Filesystem* fsys = script->filesystem;

	if (lua_gettop(L) != 1 || !lua_isstring(L, 1)) {
        throw std::exception("ChDir: invalid parameters");
	}

	Path p(lua_tostring(L, 1));
	mountId m = p.GetMountId();

	if (!p.IsValid() || p.IsFile() || m == err) {
        throw std::exception("ChDir: invalid path");
	}

	switch (m) {
		case curr:
			break;

		case boot:
			fsys->DirOpenRoot();
			break;

		default:
			// multiple drives not supported, Path returns err, so we don't need anything here.
			break;
	}

	for (size_t i = 0; i < p.DirectoryCount(); ++i) {
		if (p.GetDirectory(i) == "..") {
			fsys->DirUp();
		} else {
			fsys->DirOpen(p.GetDirectory(i).c_str());
		}
	}

	return 0;
}

// RawWrite("boot_old", 0, 0, 512)		// writes from file into filesystem
int RawWrite(lua_State* L) {
    Script* script = GetScript(L);

	if (lua_gettop(L) != 4 || !lua_isstring(L, 1) || !lua_isnumber(L, 2) || !lua_isnumber(L, 3) || !lua_isnumber(L, 4)) {
        throw std::exception("RawWrite: invalid parameters");
	}

    script->filesystem->RawWrite(lua_tostring(L, 1), lua_tointeger(L, 2), lua_tointeger(L, 3), lua_tointeger(L, 4));
	return 0;
}

// CopyIn("hello.txt", "hello.txt")
int CopyIn(lua_State* L) {
    Script* script = GetScript(L);

	if (lua_gettop(L) != 2 || !lua_isstring(L, 1) || !lua_isstring(L, 2)) {
        throw std::exception("CopyIn: invalid parameters");
	}

    script->filesystem->CopyIn(lua_tostring(L, 1), lua_tostring(L, 2));
	return 0;
}

// CopyOut("hello.txt", "hello.txt")
int CopyOut(lua_State* L) {
    Script* script = GetScript(L);

	if (lua_gettop(L) != 2 || !lua_isstring(L, 1) || !lua_isstring(L, 2)) {
        throw std::exception("CopyOut: invalid parameters");
	}

    script->filesystem->CopyOut(lua_tostring(L, 1), lua_tostring(L, 2));
	return 0;
}

// Delete("datfile.dat")
int Delete(lua_State* L) {
    Script* script = GetScript(L);

	if (lua_gettop(L) != 1 || !lua_isstring(L, 1)) {
        throw std::exception("Delete: invalid parameters");
	}

    script->filesystem->Delete(lua_tostring(L, 1));
	return 0;
}

// Rename("OHU.mgt", "datu.mgt")
int Rename(lua_State* L) {
    Script* script = GetScript(L);

	if (lua_gettop(L) != 2 || !lua_isstring(L, 1) || !lua_isstring(L, 2)) {
        throw std::exception("Rename: invalid parameters");
	}

    script->filesystem->Rename(lua_tostring(L, 1), lua_tostring(L, 2));
	return 0;
}

// DirCreate("disnewdir")
int DirCreate(lua_State* L) {
    Script* script = GetScript(L);

	if (lua_gettop(L) != 1 || !lua_isstring(L, 1)) {
        throw std::exception("DirCreate: invalid parameters");
	}

    script->filesystem->DirCreate(lua_tostring(L, 1));
	return 0;
}

// DirDelete("datdir")
int DirDelete(lua_State* L) {
    Script* script = GetScript(L);

	if (lua_gettop(L) != 1 || !lua_isstring(L, 1)) {
        throw std::exception("DirDelete: invalid parameters");
	}

    script->filesystem->DirDelete(lua_tostring(L, 1));
	return 0;
}

// Format("MOFODRIVE")
int Format(lua_State* L) {
    Script* script = GetScript(L);

	if (lua_gettop(L) != 1 || !lua_isstring(L, 1)) {
        throw std::exception("Format: invalid parameters");
	}

	script->filesystem->Format(lua_tostring(L, 1));
	return 0;
}

// Exists("file.dat")
int Exists(lua_State* L) {
    Script* script = GetScript(L);

	if (lua_gettop(L) != 1 || !lua_isstring(L, 1)) {
        throw std::exception("Exists: invalid parameters");
	}

    lua_pushboolean(L, script->filesystem->Exists(lua_tostring(L, 1)));
	return 1;
}
