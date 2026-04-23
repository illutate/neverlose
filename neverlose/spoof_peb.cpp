#include "neverlose.h"
#include "PEB_SPOOF.h"
#include "ArenaAllocator.h"
#include "HookFn.h"

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif

void neverlose::spoof_peb()
{
	clogger logger = ENTER_LOGGER(logman);

	PVOID fake_peb = load_res_to_mem(IDR_BINARY, "PEB_DATA");
	SIZE_T fake_peb_size = 0x1000;
	
	NTSTATUS status = NtAllocateVirtualMemory(NtCurrentProcess(), &fake_peb, 0, &fake_peb_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!NT_SUCCESS(status))
		panic("Failed to allocate fake PEB!");

	if (fake_peb)
		*((BYTE*)fake_peb + 0xA8) = 0x20;

	logger << "Allocated fake PEB at " << (void*)fake_peb << "\n";

    // Считаем размер массива по старинке
	ArenaAllocator<peb_spoof> peb_spoof_arena(sizeof(g_peb_spoofs) / sizeof(g_peb_spoofs[0]));

	if (!peb_spoof_arena.has_scene())
		panic("Failed to allocate PEB spoof arena!");

	for (size_t i = 0; i < sizeof(g_peb_spoofs) / sizeof(g_peb_spoofs[0]); i++)
	{
    	const peb_spoof_info& info = g_peb_spoofs[i];
    	auto* ppeb_spoof_tram = peb_spoof_arena.construct(fake_peb, info.reg);
		if (!ppeb_spoof_tram) continue;

        size_t beyond_nops = 0; // Если у тебя тут был расчет, верни его
		NTSTATUS hkstatus = HookFn((PVOID)info.address, ppeb_spoof_tram->data, beyond_nops, (PVOID*)&ppeb_spoof_tram->JumpBackAddr, 5 + beyond_nops);

		if (NT_SUCCESS(hkstatus))
			logger << "Spoofed PEB at " << (void*)info.address << "\n";
	}
}