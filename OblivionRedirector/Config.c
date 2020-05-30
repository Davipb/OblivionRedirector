#include "OR_Base.h"
#include "Config.h"
#include "Logging.h"
#include "..\Common\StringUtils.h"
#include "..\Common\WindowsUtils.h"

#include <stdlib.h>
#include <stdbool.h>
#include <ShlObj.h>
#include <Windows.h>

// The file path, relative to the current module path, where the configuration is stored
#define OR_CONFIG_FILE L"\\OblivionRedirector.ini"

// The default file path, rrelative to the current module path, where the log file is stored
#define OR_DEFAULT_LOG_FILE L"\\OblivionRedirector.log"

// The default log level, in text format
#ifdef DEBUG
#define OR_DEFAULT_LOG_LEVEL L"TRACE"
#else
#define OR_DEFAULT_LOG_LEVEL L"INFO"
#endif

// Reads a string stored in a .ini fully
// If the specified key doesn't exist, this returns null.
// Otherwise, the returned string is allocated dynamically and must be freed.
static wchar_t* OR_ReadIniString(const wchar_t* section, const wchar_t* key, const wchar_t* file)
{
	// Exponentially increase the buffer size until it fits the full module file name
	size_t resultSize = 16;
	wchar_t* result = NULL;
	DWORD actualLen;
	do
	{
		resultSize *= 2;
		result = realloc(result, resultSize * sizeof(wchar_t));
		actualLen = GetPrivateProfileStringW(section, key, NULL, result, resultSize, file);

	} while (actualLen >= resultSize - 1);

	if (actualLen == 0) // Key doesn't exist, defaults to empty string
	{
		free(result);
		return NULL;
	}

	// Trim buffer to fit string exactly
	result = realloc(result, (actualLen + 1) * sizeof(wchar_t));

	return result;
}

// Gets the file path of the configuration file.
// The returned string is allocated dynamically and must be freed.
static wchar_t* OR_GetConfigFile()
{
	wchar_t* baseDir = OR_GetModuleDirectory();
	wchar_t* result = OR_Concat(2, baseDir, OR_CONFIG_FILE);
	free(baseDir);

	return result;
}

// Gets the default file path of the log file.
// The returned string is allocated dynamically and must be freed.
static wchar_t* OR_GetDefaultLogFile()
{
	wchar_t* baseDir = OR_GetModuleDirectory();
	wchar_t* result = OR_Concat(2, baseDir, OR_DEFAULT_LOG_FILE);
	free(baseDir);

	return result;
}

// Transforms a textual log level info an integer log level
static uint8_t OR_LogStringToLogLevel(const wchar_t* level)
{
	if (OR_AreCaseInsensitiveEqualW(level, L"TRACE")) return OR_LOG_LEVEL_TRACE;
	if (OR_AreCaseInsensitiveEqualW(level, L"DEBUG")) return OR_LOG_LEVEL_DEBUG;
	if (OR_AreCaseInsensitiveEqualW(level, L"INFO")) return OR_LOG_LEVEL_INFO;
	if (OR_AreCaseInsensitiveEqualW(level, L"WARN")) return OR_LOG_LEVEL_WARN;
	if (OR_AreCaseInsensitiveEqualW(level, L"ERROR")) return OR_LOG_LEVEL_ERROR;
	return OR_LOG_LEVEL_OFF;
}

// Transforms an integer log level into a textual log level
// The returned string is static and doesn't need to be freed.
static const wchar_t* OR_LogLevelToLogString(uint8_t level)
{
	if (level == OR_LOG_LEVEL_TRACE) return L"TRACE";
	if (level == OR_LOG_LEVEL_DEBUG) return L"DEBUG";
	if (level == OR_LOG_LEVEL_INFO) return L"INFO";
	if (level == OR_LOG_LEVEL_WARN) return L"WARN";
	if (level == OR_LOG_LEVEL_ERROR) return L"ERROR";
	return L"OFF";
}

// Checks if a file is valid, that is, it exists  and is not a directory
static bool OR_IsFileValid(const wchar_t* name)
{
	DWORD attr = GetFileAttributesW(name);
	if (attr == INVALID_FILE_ATTRIBUTES) return false;

	return (attr & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

// Validates a file stored in a variable, automatically resetting it to the default value if it
// is invalid
//  name: The user-friendly name of this file
//  storage: A pointer to where the path of the file is stored.
//  getDefault: A pointer to a function that returns the default path of the file.
static void OR_ValidateFile(const wchar_t* name, wchar_t** storage, wchar_t* (*getDefault)())
{
	if (OR_IsFileValid(*storage)) return;

	OR_WARN("%ls path '%ls' doesn't exist, trying to regenerate it", name, *storage);

	free(*storage);
	*storage = getDefault();

	OR_DEBUG("Regenerated %ls path to '%ls'", name, *storage);

	if (!OR_IsFileValid(*storage))
		OR_ERROR("%ls path '%ls' is still invalid even after regeneration, redirections will fail", name, *storage);
}

static OR_UserConfig* UserConfig = NULL;

// Saves the values currently stored in UserConfig to the .ini file
static void OR_SaveUserConfig()
{
	if (UserConfig == NULL) return;
	wchar_t* configFile = OR_GetConfigFile();

	WritePrivateProfileStringW(L"Logging", L"File", UserConfig->Logging.File, configFile);
	WritePrivateProfileStringW(L"Logging", L"Level", OR_LogLevelToLogString(UserConfig->Logging.Level), configFile);

	if (UserConfig->Logging.Append)
		WritePrivateProfileStringW(L"Logging", L"Append", L"TRUE", configFile);
	else
		WritePrivateProfileStringW(L"Logging", L"Append", L"FALSE", configFile);

	free(configFile);
}

/*

This macro is to be used as follows:

  READOR("Section", "Key", GetDefaultValue());
  someVar = read;

It does the following:

  1. Call OR_ReadIniString with the variable `configFile` as the config file parameter
	 and assign its return value to the variable `read`

  2. If `read` is null, call the specified default value generator and assign it to read.

*/
#define READOR(section, key, default) \
	read = OR_ReadIniString(L##section, L##key, configFile); \
	if (read == NULL) \
		read = default;

static void OR_LoadConfig()
{
	OR_FreeUserConfig();

	UserConfig = calloc(1, sizeof(OR_UserConfig));

	wchar_t* configFile = OR_GetConfigFile();
	wchar_t* read;

	READOR("Logging", "File", OR_GetDefaultLogFile());
	UserConfig->Logging.File = read;

	READOR("Logging", "Level", _wcsdup(OR_DEFAULT_LOG_LEVEL));
	UserConfig->Logging.Level = OR_LogStringToLogLevel(read);
	free(read);

	READOR("Logging", "Append", _wcsdup(L"TRUE"));
	UserConfig->Logging.Append = OR_AreCaseInsensitiveEqualW(read, L"TRUE");
	free(read);

	free(configFile);

	OR_SaveUserConfig();
}

#undef READOR

OR_UserConfig* OR_GetUserConfig()
{
	if (UserConfig == NULL) OR_LoadConfig();
	return UserConfig;
}

void OR_ValidateUserConfig()
{
	if (UserConfig == NULL) OR_LoadConfig();

	OR_ValidateFile(L"Log File", &UserConfig->Logging.File, &OR_GetDefaultLogFile);
	OR_SaveUserConfig();
}

void OR_FreeUserConfig()
{
	if (UserConfig == NULL) return;

	free(UserConfig->Logging.File);
	free(UserConfig);

	UserConfig = NULL;
}
