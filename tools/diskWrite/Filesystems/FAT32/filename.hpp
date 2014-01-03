/*
 *	filename.hpp
 *	Filename specific functions
 */

#ifndef __FILENAME_HPP
#define __FILENAME_HPP

#include <iostream>
#include "common.hpp"

inline unsigned char Dos83Checksum(unsigned char* name);
void StripDos83IllegalCharacters(char* buffer);
void ToDos83Name(const char* name, char* buffer);
void ToTypicalName(const char* name, char* buffer);

#endif
