/*
 *	filename.cpp
 *	Filename specific functions
 */

#include <iostream>
#include "filename.hpp"

inline unsigned char Dos83Checksum(unsigned char* name) {
    short len;
    unsigned char value = 0;

    for (len = 11; len != 0; len--)
        value = ((value & 1) ? 0x80 : 0) + (value >> 1) + *name++;

    return value;
}

void StripDos83IllegalCharacters(char* buffer) {
    char illChars[] = { 0x22, 0x2A, 0x2B, 0x2C, 0x2E, 0x2F, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x5B, 0x5C, 0x5D, 0x7C };

    size_t len = strlen(buffer);
    for (size_t i = 0; i < len; i++)
    for (int j = 0; j < sizeof(illChars); j++)
    if (buffer[i] == illChars[j])
        buffer[i] = '_';
}

void ToDos83Name(const char* name, char* buffer) {
    // Copy name into a temp buffer
    size_t nameLen = strlen(name);
    char* tmp = new char[nameLen];

    memcpy(tmp, name, nameLen);
    for (int i = 0; i < DOS83_MAX_LEN; i++)
        buffer[i] = ' ';

    // Capitalize everything
    for (size_t i = 0; i < nameLen; i++)
        tmp[i] = toupper(tmp[i]);

    // Add ~1
    char* offset = strchr(tmp, '.');
    if ((offset && (offset - tmp) > 8) || (!offset && nameLen + 3 > DOS83_MAX_LEN)) {
        tmp[6] = '~';
        tmp[7] = '1';
    }

    // Move into buffer
    if (offset) {
        if ((offset - tmp) > 8)
            memcpy(buffer, tmp, 8);
        else
            memcpy(buffer, tmp, (offset - tmp));

        memcpy(buffer + 8, offset + 1, nameLen - (offset - tmp) - 1);
    }
    else {
        memcpy(buffer, tmp, nameLen);
        memcpy(buffer + 8, "   ", 3);
    }

    buffer[11] = 0;

    // Clean up
    delete[] tmp;
}

void ToTypicalName(const char* name, char* buffer) {
    // Copy name into a temp buffer
    size_t nameLen = strlen(name);
    char* tmp = new char[nameLen + 1];

    memcpy(tmp, name, nameLen);
    for (int i = 0; i < DOS83_MAX_LEN; i++)
        buffer[i] = 0;

    // Lowercase everything
    for (size_t i = 0; i < nameLen; i++)
        tmp[i] = tolower(tmp[i]);

    // Find a place to put the .
    char* offset = strchr(tmp, ' ');
    if (!offset) {
        offset = tmp + 8;
        memmove(tmp + 9, tmp + 8, 3);
        tmp[8] = ' ';
    }

    char* offset2 = offset;
    while (*offset2++ == ' ');

    if (offset)
        *offset = '.';

    // Copy over the filename and extention around the .
    memcpy(buffer, tmp, offset - tmp + 1);
    memcpy(buffer + (offset - tmp) + 1, tmp + (offset2 - tmp) - 1, 3);

    // Close off any spaces in the extention
    offset = strchr(buffer, ' ');
    if (offset)
        *offset = 0;

    buffer[12] = 0;

    // Clean up
    delete[] tmp;
}
