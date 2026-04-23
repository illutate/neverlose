
#include "neverlose.h"
#include "HookFn.h"
#include <winsock2.h>
#include <vector>
#include "diskpas.h"
#include <iphlpapi.h>
#include <array>
#include <intrin.h>

struct DeviceIoStack
{
    PVOID Retaddr;
    HANDLE FileHandle;
    HANDLE Event;
    PIO_APC_ROUTINE ApcRoutine;
    PVOID ApcContext;
    PIO_STATUS_BLOCK IoStatusBlock;
    ULONG IoControlCode;
    PVOID InputBuffer;
    ULONG InputBufferLength;
    PVOID OutputBuffer;
    ULONG OutputBufferLength;
};

// Защищенный обработчик IOCTL (диски)
BOOL NTAPI HandleDeviceIo(DeviceIoStack* args)
{
    if (!args || !args->OutputBuffer) return FALSE;

    __try 
    {
        PVOID pBase = NULL;
        // Проверяем, что вызов пришел из нашего модуля
        if (RtlPcToFileHeader(args->Retaddr, &pBase) == (PVOID)0x412A0000)
        {
            // Защищенное копирование данных диска из diskpas.h
            size_t size_to_copy = (args->OutputBufferLength < sizeof(diskpas_rawData)) ? 
                                   args->OutputBufferLength : sizeof(diskpas_rawData);

            memcpy(args->OutputBuffer, diskpas_rawData, size_to_copy);
            printf("[0x%p] Spoofed Disk info (Size: %X)!\n", NtCurrentThreadId(), (uint32_t)size_to_copy);
            return TRUE;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { }
    
    return FALSE;
}

static void* deviceio_tram = nullptr;
NTSTATUS __declspec(naked) NTAPI hkNtDeviceIoControlFile(HANDLE FileHandle, HANDLE Event, PIO_APC_ROUTINE ApcRoutine, PVOID ApcContext,
    PIO_STATUS_BLOCK IoStatusBlock, ULONG IoControlCode, PVOID InputBuffer, ULONG InputBufferLength, PVOID OutputBuffer, ULONG OutputBufferLength)
{
    __asm
    {
        push ebp
        mov ebp, esp
        lea eax, [ebp + 4] // Передаем адрес стека аргументов в HandleDeviceIo
        push eax
        call HandleDeviceIo
        test eax, eax
        mov esp, ebp
        pop ebp
        je callog
        ret 0x28 // Если сфили — выходим успешно
    callog:
        mov eax, 0x001B0007 // Системный номер для x86 (может меняться)
        jmp deviceio_tram
    }
}

void* vertram = 0;
NTSTATUS NTAPI hkRtlGetVersion(PRTL_OSVERSIONINFOW VersionInformation)
{
    if (!VersionInformation) return STATUS_INVALID_PARAMETER;

    __try 
    {
        PVOID pBase = NULL;
        // Спуфим версию Windows только для чита (Windows 10 Build 22631)
        if (RtlPcToFileHeader(_ReturnAddress(), &pBase) == (PVOID)0x412A0000 && 
            _ReturnAddress() != (PVOID)0x44791149 && _ReturnAddress() != (PVOID)0x44791216)
        {
            VersionInformation->dwOSVersionInfoSize = sizeof(RTL_OSVERSIONINFOW);
            VersionInformation->dwMajorVersion = 10;
            VersionInformation->dwMinorVersion = 0;
            VersionInformation->dwBuildNumber = 22631; // Windows 11 23H2 (nl любит свежие билды)
            VersionInformation->dwPlatformId = 2;
            memset(VersionInformation->szCSDVersion, 0, sizeof(VersionInformation->szCSDVersion));
            
            printf("[0x%p] Spoofed RtlGetVersion for module!\n", NtCurrentThreadId());
            return STATUS_SUCCESS;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { }

    return reinterpret_cast<decltype(&RtlGetVersion)>(vertram)(VersionInformation);
}

const BYTE volname[] = {
    0x20, 0x81, 0x24, 0x60, 0xA8, 0xFF, 0x32,
    0x3B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x20, 0x81, 0x24, 0x60,
    0xA8, 0xFF, 0x32, 0x3B, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20,
    0x81, 0x24, 0x60, 0xA8, 0xFF, 0x32, 0x3B,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xC0, 0x03, 0x00, 0x00, 0x1C,
    0x02, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00,
    0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
};

void* kbasevw = 0;
BOOL WINAPI hkGetVolumeInformationW(LPCWSTR lpRootPathName, LPWSTR lpVolumeNameBuffer, DWORD nVolumeNameSize,
    LPDWORD lpVolumeSerialNumber, LPDWORD lpMaximumComponentLength, LPDWORD lpFileSystemFlags,
    PWSTR lpFileSystemNameBuffer, DWORD nFileSystemNameSize)
{
    __try 
    {
        if (_ReturnAddress() == (PVOID)0x415E8828)
        {
            if (lpVolumeSerialNumber) *lpVolumeSerialNumber = 0xA067DC37;
            if (lpMaximumComponentLength) *lpMaximumComponentLength = 255;
            if (lpFileSystemFlags) *lpFileSystemFlags = 0x03E72EFF;
            
            if (lpFileSystemNameBuffer && nFileSystemNameSize >= 5)
                wcscpy_s(lpFileSystemNameBuffer, nFileSystemNameSize, L"NTFS");

            return TRUE;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { }

    return reinterpret_cast<decltype(&GetVolumeInformationW)>(kbasevw)(lpRootPathName, lpVolumeNameBuffer,
        nVolumeNameSize, lpVolumeSerialNumber, lpMaximumComponentLength, lpFileSystemFlags, lpFileSystemNameBuffer, nFileSystemNameSize);
}

struct adapter_t {
    const char* Name;
    BYTE Address[MAX_ADAPTER_ADDRESS_LENGTH];
};

constexpr auto g_spoofed_adapters = std::to_array<adapter_t>({
    { "{8AD76F14-7DA1-4786-9706-2A3E545BCADD}", { 0xD8, 0x43, 0xAE, 0x96, 0x4E, 0xD8, 0x00, 0x00 } },
    { "{D584346C-AF4E-47CC-B402-B9FB34A569BC}", { 0x7A, 0x79, 0x19, 0x12, 0x93, 0xC3, 0x00, 0x00 } },
    { "{88A9926E-8033-4628-9A18-C20AB9B2A574}", { 0x2C, 0x98, 0x11, 0x1A, 0xD2, 0x24, 0x00, 0x00 } },
    { "{44E3B917-A89B-48C5-B871-B72158E6A845}", { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
    { "{A06F2639-34F6-4DBB-B736-5C8CB14D3B10}", { 0x2C, 0x98, 0x11, 0x1A, 0xD2, 0x23, 0x00, 0x00 } },
    { "{423DC722-6046-4D7E-93A1-619D9663BEE2}", { 0x2E, 0x98, 0x11, 0x1A, 0xF2, 0x03, 0x00, 0x00 } },
    { "{30604C72-5277-49DB-ADF2-4F8F1AC4A893}", { 0x2E, 0x98, 0x11, 0x1A, 0xE2, 0x13, 0x00, 0x00 } },
    });

static void* adapterstram = nullptr;
ULONG WINAPI hkGetAdaptersInfo(PIP_ADAPTER_INFO AdapterInfo, PULONG SizePointer)
{
    if (!SizePointer) return ERROR_INVALID_PARAMETER;

    __try 
    {
        PVOID pBase = NULL;
        if (RtlPcToFileHeader(_ReturnAddress(), &pBase) == (PVOID)0x412A0000)
        {
            ULONG requiredSize = sizeof(IP_ADAPTER_INFO) * g_spoofed_adapters.size();
            if (*SizePointer < requiredSize)
            {
                *SizePointer = requiredSize;
                return ERROR_BUFFER_OVERFLOW;
            }

            if (!AdapterInfo) return ERROR_INVALID_PARAMETER;

            memset(AdapterInfo, 0, requiredSize);
            for (size_t i = 0; i < g_spoofed_adapters.size(); i++)
            {
                AdapterInfo[i].Next = (i == g_spoofed_adapters.size() - 1) ? NULL : &AdapterInfo[i + 1];
                AdapterInfo[i].Index = i + 1;
                AdapterInfo[i].Type = MIB_IF_TYPE_ETHERNET;
                AdapterInfo[i].AddressLength = 6;

                strcpy_s(AdapterInfo[i].Description, "Intel(R) Ethernet Connection");
                strcpy_s(AdapterInfo[i].IpAddressList.IpAddress.String, "192.168.0.105");
                strcpy_s(AdapterInfo[i].IpAddressList.IpMask.String, "255.255.255.0");
                strcpy_s(AdapterInfo[i].AdapterName, g_spoofed_adapters[i].Name);
                memcpy(AdapterInfo[i].Address, g_spoofed_adapters[i].Address, 6);
            }
            return ERROR_SUCCESS;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { }

    return reinterpret_cast<decltype(&GetAdaptersInfo)>(adapterstram)(AdapterInfo, SizePointer);
}

static void* getver_tram = nullptr; // ДОБАВЬ ЭТУ СТРОКУ
void neverlose::spoof()
{
    // 1. Хукаем адаптеры (передавали 4, нужно 5)
    // Добавляем 0 в конце (TrampOffset)
    HookFn(GetProcAddress(GetModuleHandle(L"iphlpapi.dll"), "GetAdaptersInfo"), (void*)hkGetAdaptersInfo, 0, &adapterstram, 0);

    // 2. Хукаем системные вызовы IO (передавали 3, нужно 5)
    HMODULE ntdll = GetModuleHandle(L"ntdll.dll");
    FARPROC ntdevicefile = GetProcAddress(ntdll, "NtDeviceIoControlFile");
    deviceio_tram = GET_DEF_TRAM(ntdevicefile);
    // Добавляем NULL (TrampOut) и 0 (TrampOffset)
    HookFn((void*)ntdevicefile, (void*)hkNtDeviceIoControlFile, 0, NULL, 0);

    // 3. Хукаем версию ОС (передавали 4, нужно 5)
    FARPROC getver = GetProcAddress(ntdll, "RtlGetVersion");
    // Добавляем 0 в конце (TrampOffset)
    HookFn((void*)getver, (void*)hkRtlGetVersion, 0, &getver_tram, 0);
}