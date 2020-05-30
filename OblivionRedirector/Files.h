#pragma once
#include <string.h>

typedef struct {
	wchar_t* IniW;
	wchar_t* PrefsIniW;
	wchar_t* PluginsW;

	char* IniA;
	char* PrefsIniA;
	char* PluginsA;
} OR_NewFiles;

typedef struct {
	wchar_t* PluginsW;
	char* PluginsA;
} OR_OldFiles;


// Initializes the file paths required by the application
// Required in order to call OR_GetNewFiles or OR_GetOldFiles
void OR_InitFiles(void);

// Gets new file paths the library should redirect calls to
// If called before OR_InitFiles or after OR_FreeFiles, its return is undefined.
OR_NewFiles* OR_GetNewFiles(void);

// Gets the old file paths the library should redirect calls away from
// If called before OR_InitFiles or after OR_FreeFiles, its return is undefined.
OR_OldFiles* OR_GetOldFiles(void);

// Frees all memory associated with file paths
// OR_GetNewFiles and OR_GetOldFiles cannot be called after this without another call to OR_InitFiles
void OR_FreeFiles(void);
