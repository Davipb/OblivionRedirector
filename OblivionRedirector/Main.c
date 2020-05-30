#include "OR_Base.h"
#include "Logging.h"
#include "Redirector.h"
#include "Config.h"
#include "Files.h"

#include <Windows.h>
#include <stdbool.h>


BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved)
{
	(void)hinst;
	(void)reserved;

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		OR_StartLogging();
		OR_DEBUG("DLL Attached");

		OR_ValidateUserConfig();
		OR_InitFiles();
		return OR_AttachRedirector();
	}
	
	if (dwReason == DLL_PROCESS_DETACH)
	{
		OR_DEBUG("DLL Detached");

		bool result = OR_DetachRedirector();
		OR_StopLogging();
		OR_FreeUserConfig();
		OR_FreeFiles();

		return result;
	}

	return TRUE;
}
