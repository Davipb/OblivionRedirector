#include <Windows.h>
#include <stdio.h>

int main()
{
	LoadLibraryW(L"OBLIVIONREDIRECTOR.DLL");
	printf("Hello");
	Sleep(10000);
}
