#include "neverlose.h"
#include "nops.h"
#include "internal_fixes.h"
#include "token.h"

// 1. Используем обычный массив, так как некоторые старые компиляторы (v145) могут иметь проблемы с std::to_array
static const DWORD g_flow_fixes_local[] = {
    0x413DCD84,
    0x413E1A6F,
    0x413E472F,
    0x413E82AD,
    0x413E9204,
    0x413EBC8C,
    0x413F3D2F,
    0x413F838F,
    0x414000DB,
    0x414084DE,
    0x41409AC1,
    0x4140FFF7,
    0x4141485B,
    0x41414BB6,
    0x41415283,
    0x41419241,
    0x414196C3,
    0x4141AB52,
    0x4141B183,
    0x4141B655,
    0x4141BAB3,
    0x4141C2E6,
    0x4141FDB5,
    0x41420111,
    0x41420484,
    0x41439012,
    0x4143DED0,
    0x4146B4E8,
    0x414703B2,
    0x4147BD0C,
    0x414802FD,
    0x4148CE6F,
    0x4148D18E,
    0x4148D4A7,
    0x41493FCF,
    0x41495B74,
    0x414AD482,
    0x414B2D58,
    0x414B5F87,
    0x414B9BFF,
    0x414BBBB7,
    0x414BCAF3,
    0x414BDB3A,
    0x414BEB2B,
    0x414BFB2F,
    0x414C0C6B,
    0x414C4F25,
    0x414CFAA6,
    0x414D18C9,
    0x414D4963,
    0x414D4DB0,
    0x414D51C1,
    0x414D6281,
    0x414E4372,
    0x414FAD34,
    0x41503C9B,
    0x415041AA,
    0x415045B7,
    0x41504942,
    0x4150816B,
    0x41508EA6,
    0x41509281,
    0x415097B2,
    0x4150E146,
    0x4150E532,
    0x415113B7,
    0x41511718,
    0x41511FCF,
    0x4151EED3,
    0x415714E1,
    0x4157183A,
    0x41571CEA,
    0x415965A2,
    0x415A71F6,
    0x415A7A11,
    0x415B25F8,
    0x415B4D0F,
    0x415B78F9,
    0x415B9A1D,
    0x415DFBB3,
    0x415E3ABA,
    0x4185A75B,
    0x41976247,
    0x419B0199
};

// 1. Вспомогательные функции (оставляем как есть)
template <typename T>
bool safe_write(uintptr_t addr, T value) {
    DWORD old;
    if (VirtualProtect((LPVOID)addr, sizeof(T), PAGE_EXECUTE_READWRITE, &old)) {
        __try { *(T*)addr = value; } __except (1) {}
        VirtualProtect((LPVOID)addr, sizeof(T), old, &old);
        return true;
    }
    return false;
}

bool safe_memset(uintptr_t addr, int value, size_t size) {
    DWORD old;
    if (VirtualProtect((LPVOID)addr, size, PAGE_EXECUTE_READWRITE, &old)) {
        __try { memset((void*)addr, value, size); } __except (1) {}
        VirtualProtect((LPVOID)addr, size, old, &old);
        return true;
    }
    return false;
}

// 2. Реализуем методы КЛАССА neverlose (раз он объявлен как class в neverlose.h)
// Используем синтаксис Класс::Метод

// Делаем её просто статической функцией в этом файле, а не членом класса
static void internal_fix_mem_dispatcher() {
    PVOID addr = NULL;
    SIZE_T size = 0x1000;
    if (NT_SUCCESS(NtAllocateVirtualMemory(GetCurrentProcess(), &addr, 0, &size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE))) {
        safe_write<DWORD>(0x4479EE00, (DWORD)addr + 0x1000);
    }
}

void neverlose::fix_dump()
{
    fix_imports();
    spoof_peb();
    spoof_kusd();
    spoof_cpuid();
    fix_interfaces();
    fix_cvars();
    fix_signatures();
    
    // Вызываем нашу локальную функцию вместо несуществующего метода класса
    internal_fix_mem_dispatcher();

    for (size_t i = 0; i < (sizeof(g_noped_addrs) / sizeof(g_noped_addrs[0])); i++)
    {
        // Проверь, что в nops.h поле называется nop_count (если count — замени ниже)
        safe_memset(g_noped_addrs[i].address, 0x90, g_noped_addrs[i].nop_count);
    }

    // Замени свой цикл на этот:
    for (size_t i = 0; i < (sizeof(g_flow_fixes_local) / sizeof(g_flow_fixes_local[0])); i++) 
    {
        safe_write<BYTE>(g_flow_fixes_local[i], 0xEB);
    }
}