#include "neverlose.h"
#include "cpuid_emulator.h"
#include "KUSER_SHARED_DATA_SPOOF.h"

// Оптимизированный эмулятор: используем логику для частых вызовов
void cpuid_emulator(CONTEXT* ctx)
{
    if (!ctx) return;

    DWORD leaf = ctx->Eax;
    DWORD subleaf = ctx->Ecx;

    if (leaf < 0x80000000)
    {
        switch (leaf)
        {
        case 0x00000000:
            ctx->Eax = 0x10;
            ctx->Ebx = 0x68747541;
            ctx->Ecx = 0x444D4163;
            ctx->Edx = 0x69746E65;
            return;
        case 0x00000001:
            ctx->Eax = 0x0A60F12;
            ctx->Ebx = 0x100800;
            ctx->Ecx = 0x7ED8320B;
            ctx->Edx = 0x178BFBFF;
            return;
        case 0x00000005:
            ctx->Eax = 0x40;
            ctx->Ebx = 0x40;
            ctx->Ecx = 0x3;
            ctx->Edx = 0x11;
            return;
        case 0x00000006:
            ctx->Eax = 0x4;
            ctx->Ebx = 0x0;
            ctx->Ecx = 0x1;
            ctx->Edx = 0x0;
            return;
        case 0x00000007:
            if (subleaf == 0)
            {
                ctx->Eax = 0x1;
                ctx->Ebx = 0x0F1BF97A9;
                ctx->Ecx = 0x405FCE;
                ctx->Edx = 0x10000010;
                return;
            }
            else if (subleaf == 1)
            {
                ctx->Eax = 20;
                ctx->Ebx = 0;
                ctx->Ecx = 0;
                ctx->Edx = 0;
                return;
            }
        case 0x00000002:
        case 0x00000003:
        case 0x00000004:
        case 0x00000008:
        case 0x00000009:
        case 0x0000000A:
        case 0x0000000C:
        case 0x0000000E:
            ctx->Eax = 0;
            ctx->Ebx = 0;
            ctx->Ecx = 0;
            ctx->Edx = 0;
            return;
        case 0x0000000B:
            ctx->Eax = 0x1;
            ctx->Ebx = 0x2;
            ctx->Ecx = 0x100;
            ctx->Edx = 0x6;
            return;
        case 0x0000000D:
            ctx->Eax = 0x2E7;
            ctx->Ebx = 0x980;
            ctx->Ecx = 0x988;
            ctx->Edx = 0x0;
            return;
        case 0x0000000F:
            ctx->Eax = 0x0;
            ctx->Ebx = 0x0FF;
            ctx->Ecx = 0x0;
            ctx->Edx = 0x2;
            return;
        case 0x00000010:
            ctx->Eax = 0x0;
            ctx->Ebx = 0x2;
            ctx->Ecx = 0x0;
            ctx->Edx = 0x0;
            return;
        default:
            return;
        };
    }
    else
    {
        switch (leaf)
        {
        case 0x80000009:
        case 0x8000000B:
        case 0x8000000C:
        case 0x8000000D:
        case 0x8000000E:
        case 0x8000000F:
        case 0x80000010:
        case 0x80000011:
        case 0x80000012:
        case 0x80000013:
        case 0x80000014:
        case 0x80000015:
        case 0x80000016:
        case 0x80000017:
        case 0x80000018:
        case 0x8000001C:
        case 0x80000023:
        case 0x80000024:
        case 0x80000025:
        case 0x80000027:
        case 0x80000028:
            ctx->Eax = 0;
            ctx->Ebx = 0;
            ctx->Ecx = 0;
            ctx->Edx = 0;
            return;
        case 0x80000000:
            ctx->Eax = 0x80000028;
            ctx->Ebx = 0x68747541;
            ctx->Ecx = 0x444D4163;
            ctx->Edx = 0x69746E65;
            return;
        case 0x80000001:
            ctx->Eax = 0x0A60F12;
            ctx->Ebx = 0x0;
            ctx->Ecx = 0x75C237FF;
            ctx->Edx = 0x2FD3FBFF;
            return;
        case 0x80000002:
            ctx->Eax = 0x20444D41;
            ctx->Ebx = 0x657A7952;
            ctx->Ecx = 0x2037206E;
            ctx->Edx = 0x30303737;
            return;
        case 0x80000003:
            ctx->Eax = 0x2D382058;
            ctx->Ebx = 0x65726F43;
            ctx->Ecx = 0x6F725020;
            ctx->Edx = 0x73736563;
            return;
        case 0x80000004:
            ctx->Eax = 0x2020726F;
            ctx->Ebx = 0x20202020;
            ctx->Ecx = 0x20202020;
            ctx->Edx = 0x202020;
            return;
        case 0x80000005:
            ctx->Eax = 0xFF48FF40;
            ctx->Ebx = 0xFF48FF40;
            ctx->Ecx = 0x20080140;
            ctx->Edx = 0x20080140;
            return;
        case 0x80000006:
            ctx->Eax = 0x5C002200;
            ctx->Ebx = 0x6C004200;
            ctx->Ecx = 0x4006140;
            ctx->Edx = 0x1009140;
            return;
        case 0x80000007:
            ctx->Eax = 0x0;
            ctx->Ebx = 0x3B;
            ctx->Ecx = 0x0;
            ctx->Edx = 0x6799;
            return;
        case 0x80000008:
            ctx->Eax = 0x3030;
            ctx->Ebx = 0x791EF257;
            ctx->Ecx = 0x400F;
            ctx->Edx = 0x10000;
            return;
        case 0x8000000A:
            ctx->Eax = 0x1;
            ctx->Ebx = 0x8000;
            ctx->Ecx = 0x0;
            ctx->Edx = 0x1EBFBCFF;
            return;
        case 0x80000019:
            ctx->Eax = 0xF048F040;
            ctx->Ebx = 0xF0400000;
            ctx->Ecx = 0x0;
            ctx->Edx = 0x0;
            return;
        case 0x8000001A:
            ctx->Eax = 0x6;
            ctx->Ebx = 0x0;
            ctx->Ecx = 0x0;
            ctx->Edx = 0x0;
            return;
        case 0x8000001B:
            ctx->Eax = 0xBFF;
            ctx->Ebx = 0x0;
            ctx->Ecx = 0x0;
            ctx->Edx = 0x0;
            return;
        case 0x8000001D:
            ctx->Eax = 0x4121;
            ctx->Ebx = 0x1C0003F;
            ctx->Ecx = 0x3F;
            ctx->Edx = 0x0;
            return;
        case 0x8000001E:
            ctx->Eax = 0xC;
            ctx->Ebx = 0x106;
            ctx->Ecx = 0x0;
            ctx->Edx = 0x0;
            return;
        case 0x8000001F:
            ctx->Eax = 0x1;
            ctx->Ebx = 0xB3;
            ctx->Ecx = 0x0;
            ctx->Edx = 0x0;
            return;
        case 0x80000020:
            ctx->Eax = 0x0;
            ctx->Ebx = 0x1E;
            ctx->Ecx = 0x0;
            ctx->Edx = 0x0;
            return;
        case 0x80000021:
            ctx->Eax = 0x62FCF;
            ctx->Ebx = 0x15C;
            ctx->Ecx = 0x0;
            ctx->Edx = 0x0;
            return;
        case 0x80000022:
            ctx->Eax = 0x7;
            ctx->Ebx = 0x84106;
            ctx->Ecx = 0x3;
            ctx->Edx = 0x0;
            return;
        case 0x80000026:
            ctx->Eax = 0x1;
            ctx->Ebx = 0x2;
            ctx->Ecx = 0x100;
            ctx->Edx = 0x0C;
            return;
        default:
            return;
        }
    }
}

static __forceinline bool IsValidReadPtr(PVOID p, size_t len)
{
    if (!p) return false;
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery(p, &mbi, sizeof(mbi))) {
        return (mbi.State == MEM_COMMIT && (mbi.Protect & (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE)));
    }
    return false;
}

LONG NTAPI nl_veh(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
    if (!ExceptionInfo || !ExceptionInfo->ExceptionRecord || !ExceptionInfo->ContextRecord)
        return EXCEPTION_CONTINUE_SEARCH;

    PEXCEPTION_RECORD rec = ExceptionInfo->ExceptionRecord;
    PCONTEXT ctx = ExceptionInfo->ContextRecord;
    DWORD exception_addr = (DWORD)rec->ExceptionAddress;

    // 1. Пропускаем системные отладочные исключения
    if (rec->ExceptionCode == 0x40010006 || rec->ExceptionCode == 0x4001000A)
        return EXCEPTION_CONTINUE_EXECUTION;

    // 2. Обработка CPUID через брейкпоинты (INT 3)
    if (rec->ExceptionCode == EXCEPTION_BREAKPOINT)
    {
        for (DWORD addr : g_veh_cpuid_emus)
        {
            if (addr == exception_addr)
            {
                cpuid_emulator(ctx);
                ctx->Eip += 2; // Пропускаем нашу инструкцию (INT3 + стаб)
                return EXCEPTION_CONTINUE_EXECUTION;
            }
        }
    }

    // 3. Если ошибка НЕ в нашем модуле - игнорируем (пусть игра сама разбирается)
    if (!g_neverlose.in_range((PVOID)exception_addr))
    {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    // 4. Логирование краша нашего модуля
    char errbuff[2048];
    uint32_t addr = (uint32_t)rec->ExceptionAddress;
    int32_t addr_diff = (int32_t)addr - (int32_t)g_neverlose.base();
    
    // Безопасный сбор стека
    DWORD addrs[15] = { 0 };
    DWORD* esp_ptr = (DWORD*)ctx->Esp;
    
    __try {
        for (int i = 0, found = 0; i < 64 && found < 15; i++) {
            if (IsValidReadPtr(&esp_ptr[i], 4)) {
                DWORD val = esp_ptr[i];
                // Если похоже на адрес возврата (адрес внутри модулей)
                if (val > 0x400000 && val < 0x7FFFFFFF) {
                    addrs[found++] = val;
                }
            }
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {}

    const char* op = (rec->ExceptionInformation[0] == 0) ? "READ" : (rec->ExceptionInformation[0] == 1 ? "WRITE" : "EXECUTE");

    sprintf_s(errbuff, sizeof(errbuff), 
        "UwuSense has encountered a fatal error!\n\n"
        "Exception: 0x%08X at 0x%08X (Offset: %c0x%X)\n"
        "Operation: %s at 0x%08X\n\n"
        "Registers:\nEAX: 0x%08X | EBX: 0x%08X\nECX: 0x%08X | EDX: 0x%08X\nESI: 0x%08X | EDI: 0x%08X\nEIP: 0x%08X | ESP: 0x%08X\n\n"
        "Call Stack (Top 5):\n1: 0x%08X\n2: 0x%08X\n3: 0x%08X\n4: 0x%08X\n5: 0x%08X",
        rec->ExceptionCode, addr, (addr_diff < 0 ? '-' : '+'), abs(addr_diff),
        op, (uint32_t)rec->ExceptionInformation[1],
        ctx->Eax, ctx->Ebx, ctx->Ecx, ctx->Edx, ctx->Esi, ctx->Edi, ctx->Eip, ctx->Esp,
        addrs[0], addrs[1], addrs[2], addrs[3], addrs[4]
    );

    MessageBoxA(NULL, errbuff, "UwuSense Crash Handler", MB_ICONERROR | MB_TOPMOST);
    
    // Вместо бесконечного цикла — завершаем процесс корректно
    TerminateProcess(GetCurrentProcess(), 0);
    return EXCEPTION_CONTINUE_SEARCH;
}

void neverlose::set_veh()
{
    // Устанавливаем VEH первым в списке (1), чтобы перехватывать ошибки раньше игры
    AddVectoredExceptionHandler(1, nl_veh);
}