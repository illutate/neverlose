#include "neverlose.h"
#include "KUSER_SHARED_DATA_SPOOF.h"
#include "ArenaAllocator.h"
#include "HookFn.h"

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif

void neverlose::spoof_kusd()
{
    PVOID kuser = load_res_to_mem(IDR_KUSER_SHARED, "KUSER_SHARED_DATA");
    if (!kuser) return;

    // Считаем количество элементов через макрос, чтобы не зависеть от типа
    size_t spoof_count = sizeof(g_kuser_spoofs) / sizeof(g_kuser_spoofs[0]);

    ArenaAllocator<kuser_data_spoof> kuser_arena(spoof_count);
    if (!kuser_arena.has_scene()) return;

    for (size_t i = 0; i < spoof_count; i++)
    {
        // Используем auto&, чтобы компилятор сам вывел тип из массива g_kuser_spoofs
        auto& info = g_kuser_spoofs[i];
        
        // Передаем параметры в конструктор
        kuser_data_spoof* pspoof_block = kuser_arena.construct(info.reg, kuser); 
        
        if (!pspoof_block) continue;

        HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
        if (ntdll) {
            FARPROC target = GetProcAddress(ntdll, "NtQuerySystemInformation");
            if (target) {
                // ВАЖНО: Вызываем HookFn строго с 5 аргументами, как в HookFn.h
                // Dst, Src, NopBytes, TrampOut, TrampOffset
                HookFn((void*)target, (void*)pspoof_block, 0, NULL, 0);
            }
        }
    }
}