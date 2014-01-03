#include <Filesystem.hpp>

Filesystem::Filesystem(const std::string& dll) {
	hm = LoadLibraryA(dll.c_str());

	_Open			= NULL;
	_Close			= NULL;
	_Exists			= NULL;
	_DirOpenRoot	= NULL;
	_DirOpen		= NULL;
	_DirUp			= NULL;
	_Copy			= NULL;
	_Rename			= NULL;
	_Delete			= NULL;
	_DirCreate		= NULL;
	_DirDelete		= NULL;
	_RawWrite		= NULL;
	_CopyIn			= NULL;
	_CopyOut		= NULL;
	_Format			= NULL;

	if (hm) {
        _LastError      = (const char*(*)())                                    GetProcAddress(hm, "LastError");
		_Open			= (bool(*)(const char*, unsigned int, unsigned int))    GetProcAddress(hm, "Open");
		_Close			= (bool(*)())									        GetProcAddress(hm, "Close");
		_Exists			= (bool(*)(const char*, bool*))						    GetProcAddress(hm, "Exists");
		_DirOpenRoot	= (bool(*)())									        GetProcAddress(hm, "DirOpenRoot");
		_DirOpen		= (bool(*)(const char*))						        GetProcAddress(hm, "DirOpen");
		_DirUp			= (bool(*)())									        GetProcAddress(hm, "DirUp");
		_Copy			= (bool(*)(const char*, const char*))			        GetProcAddress(hm, "Copy");
		_Rename			= (bool(*)(const char*, const char*))			        GetProcAddress(hm, "Rename");
		_Delete			= (bool(*)(const char*))						        GetProcAddress(hm, "Delete");
		_DirCreate		= (bool(*)(const char*))						        GetProcAddress(hm, "DirCreate");
		_DirDelete		= (bool(*)(const char*))						        GetProcAddress(hm, "DirDelete");
		_RawWrite		= (bool(*)(const char*, size_t, size_t, size_t))        GetProcAddress(hm, "RawWrite");
		_CopyIn			= (bool(*)(const char*, const char*))			        GetProcAddress(hm, "CopyIn");
		_CopyOut		= (bool(*)(const char*, const char*))			        GetProcAddress(hm, "CopyOut");
		_Format			= (bool(*)(const char*, const char*))			        GetProcAddress(hm, "Format");
	}
}

Filesystem::~Filesystem() {
	if (hm)
		FreeLibrary(hm);
}

bool Filesystem::IsLoaded() {
	return hm && _LastError && _Open && _Close;
}

void Filesystem::Open(const char* fname, unsigned int partOffset, unsigned int partLen) {
    if(!hm)
        throw std::exception("Filesystem driver not loaded");

    if (!_Open)
        throw std::exception("Filesystem does not support Open");

    if (!_Open(fname, partOffset, partLen))
        throw std::exception(_LastError());
}

void Filesystem::Close() {
    if (!hm)
        throw std::exception("Filesystem driver not loaded");

    if (!_Close)
        throw std::exception("Filesystem does not support Close");

	if (!_Close())
        throw std::exception(_LastError());
}

bool Filesystem::Exists(const char* name) {
    if (!hm)
        throw std::exception("Filesystem driver not loaded");

    if (!_Exists)
        throw std::exception("Filesystem does not support Exists");

    bool result;
    if (!_Exists(name, &result))
        throw std::exception(_LastError());
    return result;
}

void Filesystem::DirOpenRoot() {
    if (!hm)
        throw std::exception("Filesystem driver not loaded");

    if (!_DirOpenRoot)
        throw std::exception("Filesystem does not support DirOpenRoot");

    if (!_DirOpenRoot())
        throw std::exception(_LastError());
}

void Filesystem::DirOpen(const char* dir) {
    if (!hm)
        throw std::exception("Filesystem driver not loaded");

    if (!_DirOpen)
        throw std::exception("Filesystem does not support DirOpen");

    if (!_DirOpen(dir))
        throw std::exception(_LastError());
}

void Filesystem::DirUp() {
    if (!hm)
        throw std::exception("Filesystem driver not loaded");

    if (!_DirUp)
        throw std::exception("Filesystem does not support DirUp");

    if (!_DirUp())
        throw std::exception(_LastError());
}

void Filesystem::Copy(const char* source, const char* dest) {
    if (!hm)
        throw std::exception("Filesystem driver not loaded");

    if (!_Copy)
        throw std::exception("Filesystem does not support Copy");

    if (!_Copy(source, dest))
        throw std::exception(_LastError());
}

void Filesystem::Rename(const char* oldName, const char* newName) {
    if (!hm)
        throw std::exception("Filesystem driver not loaded");

    if (!_Rename)
        throw std::exception("Filesystem does not support Rename");

    if (!_Rename(oldName, newName))
        throw std::exception(_LastError());
}

void Filesystem::Delete(const char* name) {
    if (!hm)
        throw std::exception("Filesystem driver not loaded");

    if (!_Delete)
        throw std::exception("Filesystem does not support Delete");

    if (!_Delete(name))
        throw std::exception(_LastError());
}

void Filesystem::DirCreate(const char* name) {
    if (!hm)
        throw std::exception("Filesystem driver not loaded");

    if (!_DirCreate)
        throw std::exception("Filesystem does not support DirCreate");

    if (!_DirCreate(name))
        throw std::exception(_LastError());
}

void Filesystem::DirDelete(const char* name) {
    if (!hm)
        throw std::exception("Filesystem driver not loaded");

    if (!_DirDelete)
        throw std::exception("Filesystem does not support DirDelete");

    if (!_DirDelete(name))
        throw std::exception(_LastError());
}

void Filesystem::RawWrite(const char* fname, size_t offsetIn, size_t offset, size_t length) {
    if (!hm)
        throw std::exception("Filesystem driver not loaded");

    if (!_RawWrite)
        throw std::exception("Filesystem does not support RawWrite");

    if (!_RawWrite(fname, offsetIn, offset, length))
        throw std::exception(_LastError());
}

void Filesystem::CopyIn(const char* source, const char* dest) {
    if (!hm)
        throw std::exception("Filesystem driver not loaded");

    if (!_CopyIn)
        throw std::exception("Filesystem does not support CopyIn");

    if (!_CopyIn(source, dest))
        throw std::exception(_LastError());
}

void Filesystem::CopyOut(const char* source, const char* dest) {
    if (!hm)
        throw std::exception("Filesystem driver not loaded");

    if (!_CopyOut)
        throw std::exception("Filesystem does not support CopyOut");

    if (!_CopyOut(source, dest))
        throw std::exception(_LastError());
}

void Filesystem::Format(const char* fname, const char* volumeName) {
    if (!hm)
        throw std::exception("Filesystem driver not loaded");

    if (!_Format)
        throw std::exception("Filesystem does not support Format");

    if (!_Format(fname, volumeName))
        throw std::exception(_LastError());
}
