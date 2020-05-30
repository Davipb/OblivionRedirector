#pragma once
#include <shtypes.h>

// Gets a known folder for the current user.
// The returned string is allocated dynamically and must be freed.
wchar_t* OR_GetKnownFolder(const KNOWNFOLDERID* const rfid);

// Canonicizes a wide path, transforming it into an absolute path with no '.' or '..' nodes and in all uppercase
// The returned string is allocated dynamically and must be freed.
wchar_t* OR_CanonicizePathW(const wchar_t* path);

// Canonicizes a narrow path, transforming it into an absolute path with no '.' or '..' nodes and in all uppercase
// The returned string is allocated dynamically and must be freed.
char* OR_CanonicizePathA(const char* path);

// Gets the full module file path of the currently running executable.
// The returned string is allocated dynamically and must be freed.
wchar_t* OR_GetModuleFilePath(void);

// Gets the full directory path of the currently running executable.
// The returned string is allocated dynamically and must be freed.
wchar_t* OR_GetModuleDirectory(void);
