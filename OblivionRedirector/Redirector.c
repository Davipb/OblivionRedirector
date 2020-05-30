#include "OR_Base.h"
#include "Redirector.h"
#include "Redirections.h"
#include "Logging.h"

#include <stdbool.h>
#include <Windows.h>
#include "..\Detours\detours.h"


static bool Attached = false;

bool OR_AttachRedirector()
{
	if (Attached)
	{
		OR_WARN("Tried to attach redirections, but we are already attached. Ignoring.");
		return true;
	}

	Attached = true;

	OR_DEBUG("Attaching all redirections");

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	OR_Redirection* current = OR_GetRedirections();
	while (current != NULL)
	{
		DetourAttach(current->Original, current->Redirected);
		OR_TRACE("Attached %ls", current->Name);
		current = current->Next;
	}

	if (DetourTransactionCommit() != NO_ERROR)
	{
		OR_TRACE("Unable to commit transaction");
		OR_ERROR("Unable to attach redirections, plugin failed to load");
		return false;
	}

	OR_TRACE("Transaction commited");
	OR_INFO("Redirections attached successfully, plugin loaded");

	return true;
}

bool OR_DetachRedirector()
{
	if (!Attached)
	{
		OR_WARN("Tried to detach redirections, but we are already detached. Ignoring.");
		return true;
	}

	Attached = false;

	OR_TRACE("Detaching all redirections");

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	OR_Redirection* current = OR_GetRedirections();
	while (current != NULL)
	{
		DetourDetach(current->Original, current->Redirected);
		OR_TRACE("Detached %ls", current->Name);
		current = current->Next;
	}

	OR_FreeRedirections();

	if (DetourTransactionCommit() != NO_ERROR)
	{
		OR_ERROR("Unable to detach redirections, plugin failed to unload");
		return false;
	}

	OR_INFO("Redirections detached successfully, plugin unloaded");
	return true;
}
