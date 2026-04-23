#include "HookFn.h"

static void FixRels(PVOID Address, PVOID Trampoline)
{
    if (!Address || !Trampoline) return;

    BYTE* og = (BYTE*)Address;
    INT32 absolete = 0;

    __try 
    {
        switch (*og)
        {
        case 0xE8:
        case 0xE9:
            absolete = ((INT32)og + *(INT32*)(og + 1) + 5);
            *(INT32*)((BYTE*)Trampoline + 1) = absolete - (INT32)Trampoline - 5;
            break;
        case 0x0F:
            if (og[1] >= 0x80 && og[1] < 0x90)
            {
                absolete = ((INT32)og + *(INT32*)(og + 2) + 6);
                *(INT32*)((BYTE*)Trampoline + 2) = absolete - (INT32)Trampoline - 6;
            };
            break;
        default:
            break;
        };
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {}
};

extern "C" NTSTATUS NTAPI HookFn(void * Dst, void * Src, SIZE_T NopBytes, void ** TrampOut, size_t TrampOffset)
{
    if (!Dst || !Src) return 0xC000000D; // STATUS_INVALID_PARAMETER

    ULONG OldProto;
    PVOID baseaddr = Dst;
    SIZE_T localRegSize = 5 + NopBytes;
    NTSTATUS status = NtProtectVirtualMemory(NtCurrentProcess(), &baseaddr, &localRegSize, PAGE_EXECUTE_READWRITE, &OldProto);
    
    if (!NT_SUCCESS(status)) return status;

    __try 
    {
        if (TrampOut)
        {
            PVOID pTramp = NULL;
            SIZE_T tramp_copy_size = 5; 
            SIZE_T TrampSizeLocal = tramp_copy_size + 5;

            status = NtAllocateVirtualMemory(NtCurrentProcess(), &pTramp, 0, &TrampSizeLocal, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
            if (NT_SUCCESS(status))
            {
                memcpy(pTramp, (char*)Dst + TrampOffset, tramp_copy_size);
                FixRels((char*)Dst + TrampOffset, pTramp);

                BYTE* jmp_back_pos = (BYTE*)pTramp + tramp_copy_size;
                *jmp_back_pos = 0xE9;
                *(INT32*)(jmp_back_pos + 1) = (INT32)((BYTE*)Dst + 5) - (INT32)(jmp_back_pos + 5);

                *TrampOut = pTramp;
            }
        }

        // Установка самого прыжка
        INT32 rel32 = (INT32)Src - (INT32)Dst - 5;
        *(BYTE*)Dst = 0xE9;
        *(INT32*)((BYTE*)Dst + 1) = rel32;

        if (NopBytes)
            memset((char*)Dst + 5, 0x90, NopBytes);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        status = 0xC0000005; // STATUS_ACCESS_VIOLATION
    }

    // Восстановление прав доступа
    ULONG TempProto;
    NtProtectVirtualMemory(NtCurrentProcess(), &baseaddr, &localRegSize, OldProto, &TempProto);

    return status;
}