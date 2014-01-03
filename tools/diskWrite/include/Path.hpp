#pragma once

#include <string>
#include <vector>

typedef int mountId;
const mountId err = -100; // invalid
const mountId curr = -2;	// current ('./')
const mountId boot = -1;	// boot ('/'), requires root to be loaded
const mountId hd0 = 0;	// first hard drive
const mountId hd1 = 1;
const mountId hd2 = 2;
const mountId hd3 = 3;
const mountId od0 = 4;	// first optical disk
const mountId od1 = 5;
const mountId od2 = 6;
const mountId od3 = 7;

mountId ParseMountId(const std::string& path);
bool	IsValidName(const std::string& name);

class Path {
public:
    Path(const std::string& str) : fullPath(str), mId(err), isValid(false) { _parse1(); }
    Path(const char* str) : fullPath(str), mId(err), isValid(false) { _parse1(); }

    bool IsValid()								{ return isValid; }
    bool IsFile()								{ return fileName.length() > 0; }

    mountId			GetMountId()				{ return mId; }
    size_t			DirectoryCount()			{ return directories.size(); }
    const std::string&	GetDirectory(size_t i)		{ return directories[i]; }
    const std::string&	GetFilename()				{ return fileName; }

private:
    bool						isValid;
    std::string					fullPath;
    mountId						mId;
    std::vector<std::string>	directories;
    std::string					fileName;

    void _parse1();				// stage 1 parser, process drive information
    void _parse2(size_t start);	// stage 2 parser, extract directory and/or path name(s)
    void _parse3();				// stage 3 parser, check name(s) for validity
};
