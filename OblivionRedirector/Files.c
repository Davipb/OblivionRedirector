#include "OR_Base.h"
#include "Files.h"
#include "Logging.h"

#include "..\Common\StringUtils.h"
#include "..\Common\WindowsUtils.h"

#include <stdio.h>
#include <stdlib.h>
#include <ShlObj.h>

static OR_NewFiles* NewFiles = NULL;
static OR_OldFiles* OldFiles = NULL;

// Generates a path relative to a directory and stores the result in a wide and a narrow string
// baseDir: Base directory of the file. If NULL, uses the directory of the currently running module.
// fileName: Name of the file
// wideOutput: Pointer to a wide string that will receive the wide version of the path
// narrowOutput: Pointer to a narrow string that will receive the narrow version of the path
static void GenerateRelativePath(const wchar_t* baseDir, const wchar_t* const fileName, wchar_t** wideOutput, char** narrowOutput)
{
	wchar_t* moduleDir = NULL;
	if (baseDir == NULL)
	{
		moduleDir = OR_GetModuleDirectory();
		baseDir = moduleDir;
	}
	wchar_t* uncanonicizedPath = OR_Concat(2, baseDir, fileName);
	free(moduleDir);

	*wideOutput = OR_CanonicizePathW(uncanonicizedPath);
	free(uncanonicizedPath);

	*narrowOutput = OR_Utf16ToCodepage(*wideOutput);
}

void OR_InitFiles(void)
{
	OR_FreeFiles();

	NewFiles = calloc(1, sizeof(OR_NewFiles));

	GenerateRelativePath(NULL, L"\\OBLIVION.INI", &NewFiles->IniW, &NewFiles->IniA);
	GenerateRelativePath(NULL, L"\\OBLIVIONPREFS.INI", &NewFiles->PrefsIniW, &NewFiles->PrefsIniA);
	GenerateRelativePath(NULL, L"\\PLUGINS.TXT", &NewFiles->PluginsW, &NewFiles->PluginsA);

	OR_DEBUG("Ini will be redirected to %ls", NewFiles->IniW);
	OR_DEBUG("PrefsIni will be redirected to %ls", NewFiles->PrefsIniW);
	OR_DEBUG("Plugins will be redirected to %ls", NewFiles->PluginsW);

	OldFiles = calloc(1, sizeof(OR_NewFiles));

	wchar_t* appData = OR_GetKnownFolder(&FOLDERID_LocalAppData);
	GenerateRelativePath(appData, L"\\OBLIVION\\PLUGINS.TXT", &OldFiles->PluginsW, &OldFiles->PluginsA);
	free(appData);

	OR_DEBUG("Plugins will be redirected from %ls", OldFiles->PluginsW);
}

OR_NewFiles* OR_GetNewFiles(void) { return NewFiles; }
OR_OldFiles* OR_GetOldFiles(void) { return OldFiles; }

/*
The following macro is to be used as: FREE_SET_NULL(var);
It does two things:

  1. Free the pointer contained in var
  2. Set var to NULL

*/
#define FREE_SET_NULL(var) free(var); var = NULL

void OR_FreeFiles(void)
{
	if (NewFiles == NULL) { return; }

	FREE_SET_NULL(NewFiles->IniW);
	FREE_SET_NULL(NewFiles->PrefsIniW);
	FREE_SET_NULL(NewFiles->PluginsW);

	FREE_SET_NULL(NewFiles->IniA);
	FREE_SET_NULL(NewFiles->PrefsIniA);
	FREE_SET_NULL(NewFiles->PluginsA);

	FREE_SET_NULL(NewFiles);

	FREE_SET_NULL(OldFiles->PluginsW);
	FREE_SET_NULL(OldFiles->PluginsA);

	FREE_SET_NULL(OldFiles);
}

#undef FREE_SET_NULL
