#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
	struct
	{
		wchar_t* File;
		uint8_t Level;
		bool Append;

	} Logging;

} OR_UserConfig;

// Gets the current UserConfig
OR_UserConfig* OR_GetUserConfig();

// Validates the current UserConfig, checking if its files and values
// are correct, and automatically correcting them if they aren't
void OR_ValidateUserConfig();

// Frees all resources allocated to the user config
// Any pointers returned by OR_GetUserConfig() must be considered invalid 
// after this function is called.
void OR_FreeUserConfig();
