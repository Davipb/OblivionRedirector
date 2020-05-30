
#include "..\Common\WindowsUtils.h"

#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>

#define WIDE_LEN(x) ((sizeof(x) / sizeof(unsigned short)) - 1)

#define TARGET  L"OBLIVIONLAUNCHER.ORIGINAL.EXE"
#define PAYLOAD L"OBLIVIONREDIRECTOR.DLL"

typedef enum {
	EXIT_FAILED_CREATE_PROCESS = 1,
	EXIT_FAILED_INJECT_MEMORY,
	EXIT_FAILED_WRITE_MEMORY,
	EXIT_FAILED_CREATE_THREAD,
	EXIT_FAILED_LOAD_LIBRARY,
} ExitCode;

// Kills the process with a certain exit code
static void OR_Die(ExitCode exitCode);

// Sets the current working directory to the folder where the .exe is located at
static void OR_SetWorkingDirectory(void);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	(void)hInstance;
	(void)hPrevInstance;
	(void)pCmdLine;
	(void)nCmdShow;

	OR_SetWorkingDirectory();

	STARTUPINFOW startupInfo = { sizeof(STARTUPINFOW) };
	PROCESS_INFORMATION processInfo = { 0 };
	BOOL result = CreateProcessW(
		/*   lpApplicationName*/ TARGET,
		/*       lpCommandLine*/ NULL,
		/* lpProcessAttributes*/ NULL,
		/*  lpThreadAttributes*/ NULL,
		/*     bInheritHandles*/ FALSE,
		/*     dwCreationFlags*/ CREATE_SUSPENDED | CREATE_NEW_CONSOLE,
		/*       lpEnvironment*/ NULL,
		/*  lpCurrentDirectory*/ NULL,
		/*       lpStartupInfo*/ &startupInfo,
		/*lpProcessInformation*/ &processInfo
	);
	if (result == 0) OR_Die(EXIT_FAILED_CREATE_PROCESS);

	LPVOID injectedMemory = VirtualAllocEx(
		/*        hProcess*/ processInfo.hProcess,
		/*       lpAddress*/ NULL,
		/*          dwSize*/ sizeof(PAYLOAD),
		/*flAllocationType*/ MEM_COMMIT,
		/*       flProtect*/ PAGE_READWRITE
	);
	if (injectedMemory == NULL) OR_Die(EXIT_FAILED_INJECT_MEMORY);

	result = WriteProcessMemory(
		/*              hProcess*/ processInfo.hProcess,
		/*         lpBaseAddress*/ injectedMemory,
		/*              lpBuffer*/ PAYLOAD,
		/*                 nSize*/ sizeof(PAYLOAD),
		/*lpNumberOfBytesWritten*/ NULL
	);
	if (result == 0) OR_Die(EXIT_FAILED_WRITE_MEMORY);

	LPTHREAD_START_ROUTINE loadLibraryProc = (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleW(L"KERNEL32"), "LoadLibraryW");
	HANDLE injectionThread = CreateRemoteThread(
		/*          hProcess*/ processInfo.hProcess,
		/*lpThreadAttributes*/ NULL,
		/*       dwStackSize*/ 0,
		/*    lpStartAddress*/ loadLibraryProc,
		/*       lpParameter*/ injectedMemory,
		/*   dwCreationFlags*/ 0,
		/*        lpThreadId*/ NULL
	);
	if (injectionThread == NULL) OR_Die(EXIT_FAILED_CREATE_THREAD);

	DWORD waitResult = WaitForSingleObject(injectionThread, INFINITE);
	if (waitResult == WAIT_FAILED) OR_Die(EXIT_FAILED_LOAD_LIBRARY);

	DWORD injectionResult;
	GetExitCodeThread(injectionThread, &injectionResult);
	if ((HMODULE)injectionResult == NULL) OR_Die(EXIT_FAILED_LOAD_LIBRARY);

	CloseHandle(injectionThread);

	VirtualFreeEx(
		/*  hProcess*/ processInfo.hProcess,
		/* lpAddress*/ injectedMemory,
		/*    dwSize*/ 0,
		/*dwFreeType*/ MEM_RELEASE
	);

	ResumeThread(processInfo.hThread);

	CloseHandle(processInfo.hProcess);
	CloseHandle(processInfo.hThread);
	return EXIT_SUCCESS;
}

static void OR_Die(ExitCode exitCode)
{
	switch (exitCode)
	{
	case EXIT_FAILED_CREATE_PROCESS: fprintf(stderr, "Failed to spawn target process\n");
	case EXIT_FAILED_INJECT_MEMORY:  fprintf(stderr, "Failed to inject memory into target process\n");
	case EXIT_FAILED_WRITE_MEMORY:   fprintf(stderr, "Failed to write payload path to injected memory\n");
	case EXIT_FAILED_CREATE_THREAD:  fprintf(stderr, "Failed to create injection thread inside target process\n");
	case EXIT_FAILED_LOAD_LIBRARY:   fprintf(stderr, "Failed to inject library into target process\n");
	}
	ExitProcess(exitCode);
}

static void OR_SetWorkingDirectory(void)
{
	wchar_t* workingDir = OR_GetModuleDirectory();
	SetCurrentDirectoryW(workingDir);
	free(workingDir);
}
