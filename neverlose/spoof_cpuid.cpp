#include "neverlose.h"
#include "cpuid_emulator.h"
#include "ArenaAllocator.h"
#include "HookFn.h"

void neverlose::spoof_cpuid()
{
    auto logger = ENTER_LOGGER(logman);

    // 1. Защищенная загрузка ресурса
    PVOID cpuid_emu = load_res_to_mem(IDR_CPUID_EMU, "cpuid emulator");
    if (!cpuid_emu) {
        logger << "CRITICAL: Failed to load CPUID emulator resource!\n";
        return;
    }
    logger << "Loaded CPUID emulator at " << cpuid_emu << '\n';

    // 2. Безопасная инициализация арены
    if (g_cpuid_emus.empty()) {
        logger << "Warning: No CPUID addresses to emplace.\n";
    } else {
        __try {
            ArenaAllocator<cpuid_emu_emplacement> cpuid_emu_arena(g_cpuid_emus.size());

            for (auto& [address, nops] : g_cpuid_emus)
            {
                if (!address) continue;

                // Проверка прав доступа перед попыткой патча
                if (IsBadReadPtr((PVOID)address, 1)) {
                    logger << "Skip: Address " << (PVOID)address << " is not readable.\n";
                    continue;
                }

                auto* pcpuid_tramp = cpuid_emu_arena.construct(cpuid_emu);
                if (!pcpuid_tramp) continue;

                // Установка хука (используем 2 байта для JMP)
                NTSTATUS hkstatus = HookFn((PVOID)address, pcpuid_tramp->data, nops, (PVOID*)&pcpuid_tramp->JumpBackAddr, 2);
                
                if (NT_SUCCESS(hkstatus))
                    logger << "Emplaced CPUID emulator at " << (PVOID)address << '\n';
                else
                    logger << "Failed to emplace CPUID at " << (PVOID)address << " status: " << std::hex << hkstatus << std::dec << '\n';
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            logger << "Exception caught during CPUID emplacement!\n";
        }
    }

    // 3. Безопасная расстановка VEH-брейкпоинтов (INT 3)
    logger << "Setting up VEH CPUID hooks...\n";
    for (DWORD bp_addr : g_veh_cpuid_emus)
    {
        if (!bp_addr) continue;

        __try {
            // Перед записью 0xCC (INT 3) проверяем, можно ли писать в эту память
            PVOID page_addr = (PVOID)bp_addr;
            MEMORY_BASIC_INFORMATION mbi;
            if (VirtualQuery(page_addr, &mbi, sizeof(mbi))) 
            {
                DWORD old_prot;
                if (VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &old_prot)) 
                {
                    *(PBYTE)bp_addr = 0xCC;       // INT 3
                    *((PBYTE)bp_addr + 1) = 0x58; // POP EAX (типичный стаб для обмана сканеров)
                    
                    VirtualProtect(mbi.BaseAddress, mbi.RegionSize, old_prot, &old_prot);
                    logger << "VEH Breakpoint set at " << (PVOID)bp_addr << '\n';
                }
            }
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            logger << "Failed to set breakpoint at " << (PVOID)bp_addr << " (Access Denied)\n";
        }
    }
}