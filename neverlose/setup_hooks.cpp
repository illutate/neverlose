#include "neverlose.h" // Всегда первый!
#include <intrin.h>
#include <winsock2.h>
#include <vector>
#include "json.hpp"
#include "HookFn.h"
#include "neverlosesdk.hpp"

static void set_nl_logo(const char* name)
{
    constexpr size_t MAXLEN = 16;

    char buffer[MAXLEN] = { 0 };

    size_t len = strlen(name);
    if (len > MAXLEN)
        len = MAXLEN;

    memcpy(buffer, name, len);

    uint32_t* buff = reinterpret_cast<uint32_t*>(buffer);

    *(uint32_t*)0x4160555E = buff[0] ^ 0xD7E76FF9;
    *(uint32_t*)0x41605558 = buff[1] ^ 0xBA5A7287;
    *(uint32_t*)0x41605576 = buff[2] ^ 0x2D725D76;
    *(uint32_t*)0x41605570 = buff[3] ^ 0x4066CCAE;
}

HMODULE WaitForSingleModule(const char* module_name)
{
    HMODULE mod = nullptr;
    while (!mod)
    {
        mod = GetModuleHandleA(module_name);
        Sleep(0);
    }
    return mod;
}

void WSAAPI ProceedGetAddrInfo(PVOID retaddr, PCSTR* ppNodeName, PCSTR* ppServiceName)
{
    PVOID pBase = NULL;
    if (RtlPcToFileHeader(retaddr, &pBase) == (PVOID)0x412A0000)
    {
        printf("[0x%p] getaddrinfo(%s, %s)\n", NtCurrentThreadId(), *ppNodeName, *ppServiceName);
        *ppNodeName = "127.0.0.1";
        *ppServiceName = "30030";
    }
}

void* getaddr_tram = nullptr;
INT __declspec(naked) WSAAPI hkgetaddrinfo(PCSTR pNodeName, PCSTR pServiceName, const ADDRINFOA* pHints, PADDRINFOA* ppResult)
{
    __asm
    {
        push ebp
        mov ebp, esp
        lea eax, [ebp + 12]
        push eax
        lea eax, [ebp + 8]
        push eax
        push[ebp + 4]
        call ProceedGetAddrInfo
        mov esp, ebp
        pop ebp

        push ebp
        mov ebp, esp
        jmp getaddr_tram
    }
}

NTSTATUS hkterm(HANDLE, NTSTATUS)
{
    printf("Terminated from 0x%p\n", _ReturnAddress());
    RtlExitUserThread(STATUS_SUCCESS);
    return STATUS_SUCCESS;
}

void hkexit(int)
{
    printf("exit from 0x%p\n", _ReturnAddress());
    RtlExitUserThread(STATUS_SUCCESS);
}

void __cdecl hksignonstate()
{
    ((void(__cdecl*)())0x415DEBD0)();
}

void* quer_tram = 0;
NTSTATUS NTAPI hkNtQueryValueKey(
    HANDLE KeyHandle,
    PCUNICODE_STRING ValueName,
    KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    PVOID KeyValueInformation,
    ULONG Length,
    PULONG ResultLength
)
{
    __try 
    {
        ULONG size = 0;
        NtQueryKey(KeyHandle, KeyNameInformation, NULL, 0, &size);
        if (size)
        {
            PKEY_NAME_INFORMATION pkni = (PKEY_NAME_INFORMATION)malloc(size);
            if (pkni && NT_SUCCESS(NtQueryKey(KeyHandle, KeyNameInformation, pkni, size, &size)))
            {
                printf("[0x%p] 0x%p NtQueryValueKey(%.*ls)\n",
                    NtCurrentThreadId(),
                    _ReturnAddress(),
                    pkni->NameLength / sizeof(*pkni->Name),
                    pkni->Name);
            }
            if (pkni) free(pkni);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) 
    {
        // error read key data
    }

    // if nullptr: если хуй не инициализирован - вызываем системную функцию напрямую
    if (quer_tram == nullptr) {
        return NtQueryValueKey(KeyHandle, ValueName, KeyValueInformationClass, KeyValueInformation, Length, ResultLength);
    }

    return reinterpret_cast<decltype(&NtQueryValueKey)>(quer_tram)(
        KeyHandle, ValueName, KeyValueInformationClass, KeyValueInformation, Length, ResultLength);
}

struct WMProtectDate
{
    unsigned short wYear;
    unsigned char bMonth;
    unsigned char bDay;
};

struct VMProtectSerialNumberData
{
    int nState;
    wchar_t wUserName[256];
    wchar_t wEMail[256];
    WMProtectDate dtExpire;
    WMProtectDate dtMaxBuild;
    int bRunningTime;
    unsigned char nUserDataLength;
    unsigned char bUserData[255];
};

void __stdcall errhandl(std::exception& ec, PVOID a2)
{
    printf("[0x%p] 0x%p Throwed(0x%p): %s\n",
        NtCurrentThreadId(),
        _ReturnAddress(),
        a2,
        ec.what());
    NtSuspendProcess(NtCurrentProcess());
}

void __fastcall performmenu(neverlosesdk::gui::Menu& menu)
{
    menu.IsOpen = !menu.IsOpen;
}

void* signonstate_tram = 0;
__declspec(naked) void hkSignonState()
{
    __asm
    {
        pushad
        pushfd
        mov eax, 0x415DEBD0
        call eax
        popfd
        popad
        jmp signonstate_tram
    };
};

void* sndtram = 0;
// Хук для hksend с еррор защитой
void __fastcall hksend(void* hdl, void* edx, void* a1, void* const payload, size_t size)
{
    __try 
    {
        if (payload != nullptr)
        {
            printf("[0x%p] 0x%p client::send_wrap(0x%p, 0x%X)\n",
                NtCurrentThreadId(),
                _ReturnAddress(),
                payload,
                size);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) 
    {
        // ошибки при логах
    }

    // if nullptr: если хуй не создан - выходим из функции без вызова
    // чтобы не вызвать Access Violation
    if (sndtram == nullptr) {
        return;
    }

    reinterpret_cast<void(__thiscall*)(void*, void*, void* const, size_t)>(sndtram)(
        hdl, a1, payload, size);
}

void neverlose::setup_hooks()
{
    set_nl_logo("neverlose"); 

    HMODULE WS2 = WaitForSingleModule("ws2_32.dll");
    FARPROC getaddrinfo = GetProcAddress(WS2, "getaddrinfo");
    
    if (getaddrinfo) {
        getaddr_tram = (PBYTE)getaddrinfo + 5;
        // 5 аргументов: Dst, Src, Nops, TrampOut, TrampOffset
        HookFn(getaddrinfo, hkgetaddrinfo, 0, NULL, 0);
    }

    HMODULE ntdll = GetModuleHandle(L"ntdll.dll");
    if (ntdll) {
        FARPROC ntterm = GetProcAddress(ntdll, "NtTerminateProcess");
        if (ntterm) {
            HookFn(ntterm, hkterm, 0, NULL, 0);
        }
    }

    // Добавляем NULL, 0 там, где не нужен трамплин
    HookFn((PVOID)0x42026080, hkexit, 0, NULL, 0);
    HookFn((PVOID)0x4200A118, errhandl, 0, NULL, 0);
    HookFn((PVOID)0x415E9086, performmenu, 0, NULL, 0);
    HookFn((PVOID)0x41609C80, performmenu, 0, NULL, 0);

    // Здесь передаем указатель на трамплин (4-й) и смещение 0 (5-й)
    HookFn((PVOID)0x41C16EA0, hksend, 0, (void**)&sndtram, 0); 

    // DEFENSIVE FIX 21.04.2026
    HookFn((PVOID)0x415DCE40, hksignonstate, 0, (void**)&signonstate_tram, 0);
}
