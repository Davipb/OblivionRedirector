#include "WindowsUtils.h"
#include "StringUtils.h"

#include <string.h>
#include <ShlObj.h>

wchar_t* OR_GetKnownFolder(const KNOWNFOLDERID* const rfid)
{
	wchar_t* folderPath;
	SHGetKnownFolderPath(rfid, 0, NULL, &folderPath);

	// Copy to a standard C-duplicated string to allow the caller to free this string with `free`
	// instead of CoTaskMemFree
	wchar_t* result = _wcsdup(folderPath);
	CoTaskMemFree(folderPath);

	return result;
}

wchar_t* OR_CanonicizePathW(const wchar_t* path)
{
	// GetFullPathName returns the required buffer size if the buffer is too small (in this case, size 0)
	const DWORD canonicizedSize = GetFullPathNameW(path, 0, NULL, NULL);
	wchar_t* canonicized = calloc(canonicizedSize, sizeof(wchar_t));
	GetFullPathNameW(path, canonicizedSize, canonicized, NULL);

	// In-place uppercase path
	_wcsupr_s_l(canonicized, canonicizedSize, OR_GetInvariantLocale());

	return canonicized;
}

char* OR_CanonicizePathA(const char* path)
{
	// GetFullPathName returns the required buffer size if the buffer is too small (in this case, size 0)
	const DWORD canonicizedSize = GetFullPathNameA(path, 0, NULL, NULL);
	char* canonicized = calloc(canonicizedSize, sizeof(char));
	GetFullPathNameA(path, canonicizedSize, canonicized, NULL);

	// In-place uppercase path
	_strupr_s_l(canonicized, canonicizedSize, OR_GetInvariantLocale());

	return canonicized;
}

wchar_t* OR_GetModuleFilePath()
{
	// Exponentially increase the buffer size until it fits the full module file name
	size_t moduleFilePathSize = 16;
	wchar_t* moduleFilePath = NULL;
	do
	{
		moduleFilePathSize *= 2;
		moduleFilePath = realloc(moduleFilePath, moduleFilePathSize * sizeof(wchar_t));
		GetModuleFileNameW(NULL, moduleFilePath, moduleFilePathSize);

	} while (GetLastError() == ERROR_INSUFFICIENT_BUFFER);

	wchar_t* canonicalModuleFilePath = OR_CanonicizePathW(moduleFilePath);
	free(moduleFilePath);

	return canonicalModuleFilePath;
}

wchar_t* OR_GetModuleDirectory()
{
	wchar_t* moduleFilePath = OR_GetModuleFilePath();
	wchar_t* moduleFileDir = OR_GetDirectory(moduleFilePath);
	free(moduleFilePath);

	return moduleFileDir;
}