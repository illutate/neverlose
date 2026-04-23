#include "neverlose.h"
#include <iostream>    // Если нужен вывод
#include <stdio.h>


neverlose g_neverlose;

void neverlose::panic(const char* fmt, ...)
{
	char buffer[1024];
	va_list va;
	va_start(va, fmt);
	vsprintf_s(buffer, sizeof(buffer), fmt, va);
	va_end(va);

	MessageBoxA(0, buffer, "FATAL ERROR", MB_ICONERROR);
	TerminateProcess(GetCurrentProcess(), 0xC0000001);
};

void neverlose::map(HMODULE hModule)
{
	hThis = hModule;
	
	HRSRC hRes = FindResource(hThis, MAKEINTRESOURCE(IDR_BINARY), L"BINARY");
	if (!hRes) panic("Failed to locate cheat binary (IDR_BINARY)!");

	HGLOBAL hResData = LoadResource(hThis, hRes);
	if (!hResData) panic("Failed to load cheat binary!");

	LPVOID pData = LockResource(hResData);
	if (!pData) panic("Failed to lock cheat binary!");

	DWORD Size = SizeofResource(hThis, hRes);
	if (Size) imageSize = Size;

	NTSTATUS status = NtAllocateVirtualMemory(GetCurrentProcess(), &baseAddr, 0, &imageSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	if (status < 0)
		panic("Failed to allocate cheat base! Status: 0x%X", status);

	{
		clogger logger = ENTER_LOGGER(logman);
		logger << "Allocated cheat base at " << (void*)baseAddr << "\n";
	}

	SIZE_T bytesWritten = 0;
	if (!WriteProcessMemory(GetCurrentProcess(), baseAddr, pData, imageSize, &bytesWritten))
		panic("Failed to write cheat image! Error: %d", GetLastError());
};

PVOID neverlose::load_res_to_mem(int idr, const char* rcname) const
{
	HRSRC hRes = FindResource(hThis, MAKEINTRESOURCE(idr), L"BINARY");
	if (!hRes) panic("Failed to find %s binary!", rcname);

	HGLOBAL hResData = LoadResource(hThis, hRes);
	if (!hResData) panic("Failed to load %s binary!", rcname);

	LPVOID pData = LockResource(hResData);
	if (!pData) panic("Failed to lock %s binary!", rcname);

	DWORD Size = SizeofResource(hThis, hRes);
	PVOID addr = NULL;
	SIZE_T sz = Size;

	if (NT_SUCCESS(NtAllocateVirtualMemory(NtCurrentProcess(), &addr, 0, &sz, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE)))
	{
		memcpy(addr, pData, Size);
		return addr;
	}
	return NULL;
}