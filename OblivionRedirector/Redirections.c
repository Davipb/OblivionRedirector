#include "OR_Base.h"
#include "Redirections.h"
#include "Logging.h"
#include "Files.h"

#include "..\Common\StringUtils.h"
#include "..\Common\WindowsUtils.h"

#include <stdbool.h>

// +==================================================================+
// |                         Redirect support                         |
// +==================================================================+




// Checks if the canonical version of a wide path ends with a specified wide string.
static bool CanonicalEndsWithW(const wchar_t* path, const wchar_t* component)
{
	wchar_t* canonical = OR_CanonicizePathW(path);

	bool result = OR_EndsWithW(canonical, component);

	free(canonical);
	return result;
}

// Checks if the canonical version of a narrow path ends with a specified narrow string.
static bool CanonicalEndsWithA(const char* path, const char* component)
{
	char* canonical = OR_CanonicizePathA(path);

	bool result = OR_EndsWithA(canonical, component);

	free(canonical);
	return result;
}

// Checks if the canonical version of a wide path is equal to a specified wide string
static bool CanonicalEqualsW(const wchar_t* path, const wchar_t* other)
{
	wchar_t* canonical = OR_CanonicizePathW(path);

	bool result = wcscmp(canonical, other) == 0;

	free(canonical);
	return result;
}

// Checks if the canonical version of a narrow path is equal to a specified narrow string
static bool CanonicalEqualsA(const char* path, const char* other)
{
	char* canonical = OR_CanonicizePathA(path);

	bool result = strcmp(canonical, other) == 0;

	free(canonical);
	return result;
}

// Tries to redirect a wide path. If the path can't be redirected, it is returned unchanged.
// The returned string does not need to be freed.
static const wchar_t* TryRedirectW(const wchar_t* input)
{
	OR_TRACE("[A] Trying to redirect %ls", input);
	const wchar_t* fileName = OR_GetFileNameW(input);

	// Canonicizing a path is expensive
	// Match the file name first to avoid canonicizing a path whenever possible

	if (OR_AreCaseInsensitiveEqualW(fileName, L"OBLIVION.INI"))
	{
		if (CanonicalEndsWithW(input, L"MY GAMES\\OBLIVION\\OBLIVION.INI"))
		{
			OR_TRACE("[A] Redirected %ls -> %ls", input, OR_GetNewFiles()->IniW);
			return OR_GetNewFiles()->IniW;
		}
	}
	else if (OR_AreCaseInsensitiveEqualW(fileName, L"OBLIVIONPREFS.INI"))
	{
		if (CanonicalEndsWithW(input, L"MY GAMES\\OBLIVION\\OBLIVIONPREFS.INI"))
		{
			OR_TRACE("[A] Redirected %ls -> %ls", input, OR_GetNewFiles()->PrefsIniW);
			return OR_GetNewFiles()->PrefsIniW;
		}
	}
	else if (OR_AreCaseInsensitiveEqualW(fileName, L"PLUGINS.TXT"))
	{
		if (CanonicalEqualsW(input, OR_GetOldFiles()->PluginsW))
		{
			OR_TRACE("[A] Redirected %ls -> %ls", input, OR_GetNewFiles()->PluginsW);
			return OR_GetNewFiles()->PluginsW;
		}
	}

	return input;
}

// Tries to redirect a narrow path. If the path can't be redirected, it is returned unchanged.
// The returned string does not need to be freed.
static const char* TryRedirectA(const char* input)
{
	OR_TRACE("[W] Trying to redirect %hs", input);
	const char* fileName = OR_GetFileNameA(input);

	// Canonicizing a path is expensive
	// Match the file name first to avoid canonicizing a path whenever possible

	if (OR_AreCaseInsensitiveEqualA(fileName, "OBLIVION.INI"))
	{
		if (CanonicalEndsWithA(input, "MY GAMES\\OBLIVION\\OBLIVION.INI"))
		{
			OR_TRACE("[W] Redirected %hs -> %hs", input, OR_GetNewFiles()->IniA);
			return OR_GetNewFiles()->IniA;
		}
	}
	else if (OR_AreCaseInsensitiveEqualA(fileName, "OBLIVIONPREFS.INI"))
	{
		if (CanonicalEndsWithA(input, "MY GAMES\\OBLIVION\\OBLIVIONPREFS.INI"))
		{
			OR_TRACE("[W] Redirected %hs -> %hs", input, OR_GetNewFiles()->PrefsIniA);
			return OR_GetNewFiles()->PrefsIniA;
		}
	}
	else if (OR_AreCaseInsensitiveEqualA(fileName, "PLUGINS.TXT"))
	{
		if (CanonicalEqualsA(input, OR_GetOldFiles()->PluginsA))
		{
			OR_TRACE("[W] Redirected %hs -> %hs", input, OR_GetNewFiles()->PluginsA);
			return OR_GetNewFiles()->PluginsA;
		}
	}

	return input;
}
/*
+==================================================================+
|                        Redirect functions                        |
+==================================================================+
| The functions below are used by the game instead of the          |
| actual Windows API.                                              |
+------------------------------------------------------------------+
*/

/*
The following macro is be used as: REDIRECT(WinAPI function name, WinAPI return value, WinAPI arguments)
It creates three definitions:

  1. A typedef for a function pointer of the specified API, called (name)_t

  2. A static variable of the type just typedef'd, called OR_Original_(name)
	 This variable should store the original WinAPI being redirected, and can
	 be used to call the original API from inside the redirect.

  3. A function signature identical to the API being redirected, called
	 OR_Redirect_(name)
*/


#define REDIRECT(name, ret, ...) typedef ret(WINAPI *##name##_t)(__VA_ARGS__); \
	static name##_t OR_Original_##name; \
	ret WINAPI OR_Redirect_##name(__VA_ARGS__)

REDIRECT(CreateFileA, HANDLE, LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	lpFileName = TryRedirectA(lpFileName);
	return OR_Original_CreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

REDIRECT(CreateFileW, HANDLE, LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	lpFileName = TryRedirectW(lpFileName);
	return OR_Original_CreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

REDIRECT(OpenFile, HFILE, LPCSTR lpFileName, LPOFSTRUCT lpReOpenBuff, UINT uStyle)
{
	lpFileName = TryRedirectA(lpFileName);
	return OR_Original_OpenFile(lpFileName, lpReOpenBuff, uStyle);
}

REDIRECT(GetPrivateProfileStringA, DWORD, LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpDefault, LPSTR lpReturnedString, DWORD nSize, LPCSTR lpFileName)
{
	lpFileName = TryRedirectA(lpFileName);
	return OR_Original_GetPrivateProfileStringA(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, lpFileName);
}

REDIRECT(GetPrivateProfileStringW, DWORD, LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpDefault, LPWSTR lpReturnedString, DWORD nSize, LPCWSTR lpFileName)
{
	lpFileName = TryRedirectW(lpFileName);
	return OR_Original_GetPrivateProfileStringW(lpAppName, lpKeyName, lpDefault, lpReturnedString, nSize, lpFileName);
}

REDIRECT(GetPrivateProfileIntA, UINT, LPCSTR lpAppName, LPCSTR lpKeyName, INT nDefault, LPCSTR lpFileName)
{
	lpFileName = TryRedirectA(lpFileName);
	return OR_Original_GetPrivateProfileIntA(lpAppName, lpKeyName, nDefault, lpFileName);
}

REDIRECT(GetPrivateProfileIntW, UINT, LPCWSTR lpAppName, LPCWSTR lpKeyName, INT nDefault, LPCWSTR lpFileName)
{
	lpFileName = TryRedirectW(lpFileName);
	return OR_Original_GetPrivateProfileIntW(lpAppName, lpKeyName, nDefault, lpFileName);
}

REDIRECT(GetPrivateProfileSectionA, DWORD, LPCSTR lpAppName, LPSTR  lpReturnedString, DWORD  nSize, LPCSTR lpFileName)
{
	lpFileName = TryRedirectA(lpFileName);
	return OR_Original_GetPrivateProfileSectionA(lpAppName, lpReturnedString, nSize, lpFileName);
}

REDIRECT(GetPrivateProfileSectionW, DWORD, LPCWSTR lpAppName, LPWSTR lpReturnedString, DWORD nSize, LPCWSTR lpFileName)
{
	lpFileName = TryRedirectW(lpFileName);
	return OR_Original_GetPrivateProfileSectionW(lpAppName, lpReturnedString, nSize, lpFileName);
}

REDIRECT(GetPrivateProfileStructA, BOOL, LPCSTR lpszSection, LPCSTR lpszKey, LPVOID lpStruct, UINT   uSizeStruct, LPCSTR szFile)
{
	szFile = TryRedirectA(szFile);
	return OR_Original_GetPrivateProfileStructA(lpszSection, lpszKey, lpStruct, uSizeStruct, szFile);
}

REDIRECT(GetPrivateProfileStructW, BOOL, LPCWSTR lpszSection, LPCWSTR lpszKey, LPVOID lpStruct, UINT uSizeStruct, LPCWSTR szFile)
{
	szFile = TryRedirectW(szFile);
	return OR_Original_GetPrivateProfileStructW(lpszSection, lpszKey, lpStruct, uSizeStruct, szFile);
}

REDIRECT(GetPrivateProfileSectionNamesA, DWORD, LPSTR  lpszReturnBuffer, DWORD  nSize, LPCSTR lpFileName)
{
	lpFileName = TryRedirectA(lpFileName);
	return OR_Original_GetPrivateProfileSectionNamesA(lpszReturnBuffer, nSize, lpFileName);
}

REDIRECT(GetPrivateProfileSectionNamesW, DWORD, LPWSTR  lpszReturnBuffer, DWORD   nSize, LPCWSTR lpFileName)
{
	lpFileName = TryRedirectW(lpFileName);
	return OR_Original_GetPrivateProfileSectionNamesW(lpszReturnBuffer, nSize, lpFileName);
}

REDIRECT(WritePrivateProfileSectionA, BOOL, LPCSTR lpAppName, LPCSTR lpString, LPCSTR lpFileName)
{
	lpFileName = TryRedirectA(lpFileName);
	return OR_Original_WritePrivateProfileSectionA(lpAppName, lpString, lpFileName);
}

REDIRECT(WritePrivateProfileSectionW, BOOL, LPCWSTR lpAppName, LPCWSTR lpString, LPCWSTR lpFileName)
{
	lpFileName = TryRedirectW(lpFileName);
	return OR_Original_WritePrivateProfileSectionW(lpAppName, lpString, lpFileName);
}

REDIRECT(WritePrivateProfileStringA, BOOL, LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpString, LPCSTR lpFileName)
{
	lpFileName = TryRedirectA(lpFileName);
	return OR_Original_WritePrivateProfileStringA(lpAppName, lpKeyName, lpString, lpFileName);
}

REDIRECT(WritePrivateProfileStringW, BOOL, LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpString, LPCWSTR lpFileName)
{
	lpFileName = TryRedirectW(lpFileName);
	return OR_Original_WritePrivateProfileStringW(lpAppName, lpKeyName, lpString, lpFileName);
}

REDIRECT(WritePrivateProfileStructA, BOOL, LPCSTR lpszSection, LPCSTR lpszKey, LPVOID lpStruct, UINT   uSizeStruct, LPCSTR szFile)
{
	szFile = TryRedirectA(szFile);
	return OR_Original_WritePrivateProfileStructA(lpszSection, lpszKey, lpStruct, uSizeStruct, szFile);
}

REDIRECT(WritePrivateProfileStructW, BOOL, LPCWSTR lpszSection, LPCWSTR lpszKey, LPVOID  lpStruct, UINT    uSizeStruct, LPCWSTR szFile)
{
	szFile = TryRedirectW(szFile);
	return OR_Original_WritePrivateProfileStructW(lpszSection, lpszKey, lpStruct, uSizeStruct, szFile);
}

REDIRECT(GetFileAttributesA, DWORD, LPCSTR lpFileName)
{
	lpFileName = TryRedirectA(lpFileName);
	return OR_Original_GetFileAttributesA(lpFileName);
}

REDIRECT(GetFileAttributesW, DWORD, LPCWSTR lpFileName)
{
	lpFileName = TryRedirectW(lpFileName);
	return OR_Original_GetFileAttributesW(lpFileName);
}

REDIRECT(GetFileAttributesExA, BOOL, LPCSTR lpFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId, LPVOID lpFileInformation)
{
	lpFileName = TryRedirectA(lpFileName);
	return OR_Original_GetFileAttributesExA(lpFileName, fInfoLevelId, lpFileInformation);
}

REDIRECT(GetFileAttributesExW, BOOL, LPCWSTR lpFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId, LPVOID lpFileInformation)
{
	lpFileName = TryRedirectW(lpFileName);
	return OR_Original_GetFileAttributesExW(lpFileName, fInfoLevelId, lpFileInformation);
}

REDIRECT(SetFileAttributesA, BOOL, LPCSTR lpFileName, DWORD dwFileAttributes)
{
	lpFileName = TryRedirectA(lpFileName);
	return OR_Original_SetFileAttributesA(lpFileName, dwFileAttributes);
}

REDIRECT(SetFileAttributesW, BOOL, LPCWSTR lpFileName, DWORD dwFileAttributes)
{
	lpFileName = TryRedirectW(lpFileName);
	return OR_Original_SetFileAttributesW(lpFileName, dwFileAttributes);
}

REDIRECT(CopyFileA, BOOL, LPCSTR lpExistingFileName, LPCSTR lpNewFileName, BOOL bFailIfExists)
{
	lpExistingFileName = TryRedirectA(lpExistingFileName);
	lpExistingFileName = TryRedirectA(lpNewFileName);
	return OR_Original_CopyFileA(lpExistingFileName, lpNewFileName, bFailIfExists);
}

REDIRECT(CopyFileW, BOOL, LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists)
{
	lpExistingFileName = TryRedirectW(lpExistingFileName);
	lpExistingFileName = TryRedirectW(lpNewFileName);
	return OR_Original_CopyFileW(lpExistingFileName, lpNewFileName, bFailIfExists);
}

REDIRECT(CopyFileExA, BOOL, LPCSTR lpExistingFileName, LPCSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags)
{
	lpExistingFileName = TryRedirectA(lpExistingFileName);
	lpExistingFileName = TryRedirectA(lpNewFileName);
	return OR_Original_CopyFileExA(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
}

REDIRECT(CopyFileExW, BOOL, LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags)
{
	lpExistingFileName = TryRedirectW(lpExistingFileName);
	lpExistingFileName = TryRedirectW(lpNewFileName);
	return OR_Original_CopyFileExW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, pbCancel, dwCopyFlags);
}

REDIRECT(CreateHardLinkA, BOOL, LPCSTR lpFileName, LPCSTR lpExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	lpFileName = TryRedirectA(lpFileName);
	lpExistingFileName = TryRedirectA(lpExistingFileName);
	return OR_Original_CreateHardLinkA(lpFileName, lpExistingFileName, lpSecurityAttributes);
}

REDIRECT(CreateHardLinkW, BOOL, LPCWSTR lpFileName, LPCWSTR lpExistingFileName, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	lpFileName = TryRedirectW(lpFileName);
	lpExistingFileName = TryRedirectW(lpExistingFileName);
	return OR_Original_CreateHardLinkW(lpFileName, lpExistingFileName, lpSecurityAttributes);
}

REDIRECT(CreateSymbolicLinkA, BOOLEAN, LPCSTR lpSymlinkFileName, LPCSTR lpTargetFileName, DWORD dwFlags)
{
	lpSymlinkFileName = TryRedirectA(lpSymlinkFileName);
	lpTargetFileName = TryRedirectA(lpTargetFileName);
	return OR_Original_CreateSymbolicLinkA(lpSymlinkFileName, lpTargetFileName, dwFlags);
}

REDIRECT(CreateSymbolicLinkW, BOOLEAN, LPCWSTR lpSymlinkFileName, LPCWSTR lpTargetFileName, DWORD dwFlags)
{
	lpSymlinkFileName = TryRedirectW(lpSymlinkFileName);
	lpTargetFileName = TryRedirectW(lpTargetFileName);
	return OR_Original_CreateSymbolicLinkW(lpSymlinkFileName, lpTargetFileName, dwFlags);
}

REDIRECT(DeleteFileA, BOOL, LPCSTR lpFileName)
{
	lpFileName = TryRedirectA(lpFileName);
	return OR_Original_DeleteFileA(lpFileName);
}

REDIRECT(DeleteFileW, BOOL, LPCWSTR lpFileName)
{
	lpFileName = TryRedirectW(lpFileName);
	return OR_Original_DeleteFileW(lpFileName);
}

REDIRECT(MoveFileA, BOOL, LPCSTR lpExistingFileName, LPCSTR lpNewFileName)
{
	lpExistingFileName = TryRedirectA(lpExistingFileName);
	lpNewFileName = TryRedirectA(lpNewFileName);
	return OR_Original_MoveFileA(lpExistingFileName, lpNewFileName);
}

REDIRECT(MoveFileW, BOOL, LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName)
{
	lpExistingFileName = TryRedirectW(lpExistingFileName);
	lpNewFileName = TryRedirectW(lpNewFileName);
	return OR_Original_MoveFileW(lpExistingFileName, lpNewFileName);
}

REDIRECT(MoveFileExA, BOOL, LPCSTR lpExistingFileName, LPCSTR lpNewFileName, DWORD dwFlags)
{
	lpExistingFileName = TryRedirectA(lpExistingFileName);
	lpNewFileName = TryRedirectA(lpNewFileName);
	return OR_Original_MoveFileExA(lpExistingFileName, lpNewFileName, dwFlags);
}

REDIRECT(MoveFileExW, BOOL, LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, DWORD dwFlags)
{
	lpExistingFileName = TryRedirectW(lpExistingFileName);
	lpNewFileName = TryRedirectW(lpNewFileName);
	return OR_Original_MoveFileExW(lpExistingFileName, lpNewFileName, dwFlags);
}

REDIRECT(MoveFileWithProgressA, BOOL, LPCSTR lpExistingFileName, LPCSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, DWORD dwFlags)
{
	lpExistingFileName = TryRedirectA(lpExistingFileName);
	lpNewFileName = TryRedirectA(lpNewFileName);
	return OR_Original_MoveFileWithProgressA(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, dwFlags);
}

REDIRECT(MoveFileWithProgressW, BOOL, LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, LPPROGRESS_ROUTINE lpProgressRoutine, LPVOID lpData, DWORD dwFlags)
{
	lpExistingFileName = TryRedirectW(lpExistingFileName);
	lpNewFileName = TryRedirectW(lpNewFileName);
	return OR_Original_MoveFileWithProgressW(lpExistingFileName, lpNewFileName, lpProgressRoutine, lpData, dwFlags);
}

#undef REDIRECT

// +==================================================================+
// |                      End Redirect functions                      |
// +==================================================================+


static OR_Redirection* Redirections = NULL;

// Adds a WinAPI function redirection to the list of redirections.
static void AddRedirection(PVOID* original, PVOID redirected, const wchar_t* name)
{
	OR_Redirection* current = calloc(1, sizeof(OR_Redirection));
	current->Next = Redirections;

	current->Original = original;
	current->Redirected = redirected;
	current->Name = name;

	Redirections = current;
}

/*
The following macro is to be used as: ADD_REDIRECT(name);
It does two things:

  1. Set OR_Original_(name) to the address of (name) in Kernel32

  2. Call AddRedirection with OR_Original_(name) as the original pointer and
	 OR_Redirect_(name) as the redirect pointer.

A convenience macro, ADD_REDIRECTAW, is supplied for redirecting both the A (ANSI)
and W (Wide/Unicode) versions of a function.

*/
#define ADD_REDIRECT(name) OR_Original_##name = (name##_t)GetProcAddress(kernel32, #name); AddRedirection(&(PVOID)OR_Original_##name, (PVOID)OR_Redirect_##name, L#name)
#define ADD_REDIRECTAW(name) ADD_REDIRECT(name##A); ADD_REDIRECT(name##W)

static void CreateRedirections()
{
	OR_FreeRedirections();

	HMODULE kernel32 = GetModuleHandleW(L"kernel32");

	ADD_REDIRECTAW(CreateFile);
	ADD_REDIRECTAW(DeleteFile);
	ADD_REDIRECTAW(CopyFile);
	ADD_REDIRECTAW(CopyFileEx);
	ADD_REDIRECTAW(MoveFile);
	ADD_REDIRECTAW(MoveFileEx);
	ADD_REDIRECTAW(MoveFileWithProgress);
	ADD_REDIRECTAW(CreateHardLink);
	ADD_REDIRECTAW(CreateSymbolicLink);

	ADD_REDIRECT(OpenFile);

	ADD_REDIRECTAW(GetPrivateProfileSection);
	ADD_REDIRECTAW(GetPrivateProfileString);
	ADD_REDIRECTAW(GetPrivateProfileInt);
	ADD_REDIRECTAW(GetPrivateProfileStruct);
	ADD_REDIRECTAW(GetPrivateProfileSectionNames);

	ADD_REDIRECTAW(WritePrivateProfileSection);
	ADD_REDIRECTAW(WritePrivateProfileString);
	ADD_REDIRECTAW(WritePrivateProfileStruct);

	ADD_REDIRECTAW(GetFileAttributes);
	ADD_REDIRECTAW(GetFileAttributesEx);
	ADD_REDIRECTAW(SetFileAttributes);
}

#undef ADD_REDIRECTAW
#undef ADD_REDIRECT

OR_Redirection* OR_GetRedirections()
{
	if (Redirections == NULL) CreateRedirections();
	return Redirections;
}

void OR_FreeRedirections()
{
	while (Redirections != NULL)
	{
		OR_Redirection* previous = Redirections;
		Redirections = Redirections->Next;
		free(previous);
	}
}
