#pragma once
#include <Windows.h>

typedef struct OR_Redirection
{
	const wchar_t* Name;
	PVOID* Original;
	PVOID Redirected;

	struct OR_Redirection* Next;

} OR_Redirection;

OR_Redirection* OR_GetRedirections();
void OR_FreeRedirections();
