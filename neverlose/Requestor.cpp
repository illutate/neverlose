#include <intrin.h>

#include "internal_fixes.h"
#include "neverlosesdk.hpp"
#include "HookFn.h"
#include "token.h"
#include <cstdarg>
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

#define GHETTO_FIX

#ifdef GHETTO_FIX

static HINTERNET hSession = NULL;
static HINTERNET hConnection = NULL;

static void requestor_log(const char* fmt, ...)
{
    char path[MAX_PATH]{};
    if (!GetTempPathA(MAX_PATH, path))
        return;
    strcat_s(path, "nl_requestor_debug.log");

    HANDLE file = CreateFileA(path, FILE_APPEND_DATA, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE)
        return;

    char line[2048]{};
    va_list args;
    va_start(args, fmt);
    int len = vsprintf_s(line, fmt, args);
    va_end(args);

    if (len > 0)
    {
        DWORD written = 0;
        WriteFile(file, line, len, &written, nullptr);
    }

    CloseHandle(file);
}

void __fastcall GetSerial(void* ecx, void* edx, std::string& out, nlohmann::json& request)
{
    // Подмена серийного номера
    new (&out) std::string("g6w/cgN2AuDsLw3xrzboM1kbkLy+osvg0Y/j0LJnQf04GHbV8s5V4yReEk1mh3ZA2G72fHG3oOh7zlGEfR1nKw717WiwRwsrgSDfJtaTQz14VDDkayLBNV1DaT/qSyx8Frg1nXU0crRu1P/G+EPvH6nWNPYLZdUMIeqVCToEFhJnqiuRoAyypjFNiKnLEMiy5j2YvBcLCOC8yC3FPt/GGsvUldBqkmQGkBjIsXsSkut05txVxq7VDx1i9adKE4zalTzNHr0Vtd6DTr8aeH8NYHWPGWAsnTBkZlkNuRuhBTtgRTcIKxzGATTN4k8/JaXCpxri7IqsylvZgXQw+5zldLjAHqcAWw3OD5iQn8DtOoon+DrHm3k3FY6wIrCM1FzTdjAIcTvXSiWOURHiwA4sJ8ExR4dyBZMydo8aBAYjrRxcD9oDa/VVJT4cZfDkyWvRjI3WMyEajF2JhiGcjpjztmD8fyt9C16VXwLfoYuJnrX1/Dv8SZfCU6U2UhwJlxO5mkg+/IctveCdxy8IIiXTKwA5vmiEpXRuUu17SCdmJhFLZ+Jr6cTmrob4exSEggGRk6BTaVomOq4I6IpkVUBIUVup+4JvWFseL5UkPOQqHIO5Rxnj1jY+PjAWFPeeXSZsP8/ceEnX8J13tfb7PAqRSrpQ1Wv/y+OjaqMoPg9PiRE=");
    printf("Spoofed serial %s\n", request.dump().c_str());
}

void __fastcall MakeRequest(void* ecx, void* edx, std::string& out, std::string_view route, int _, int __)
{
    printf("[0x%p] 0x%p MakeRequest(%.*s, 0x%X, 0x%X) spoofed\n",
        NtCurrentThreadId(), _ReturnAddress(), (int)route.size(), route.data(), _, __);
    
    requestor_log("[0x%p] 0x%p raw MakeRequest(%.*s, 0x%X, 0x%X)\n",
        NtCurrentThreadId(), _ReturnAddress(), (int)route.size(), route.data(), _, __);

    new (&out) std::string("");

    int size_needed = MultiByteToWideChar(CP_UTF8, 0, route.data(), (int)route.size(), NULL, 0);
    wchar_t* wroute = (wchar_t*)malloc((size_needed + 1) * sizeof(wchar_t));

    if (wroute)
    {
        MultiByteToWideChar(CP_UTF8, 0, route.data(), (int)route.size(), wroute, size_needed);
        wroute[size_needed] = L'\0';
        HINTERNET hRequest = WinHttpOpenRequest(hConnection, L"GET", wroute, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
        free(wroute);
        if (hRequest && WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0))
        {
            if (WinHttpReceiveResponse(hRequest, NULL))
            {
                DWORD dwSize = 0;
                DWORD dwDownloaded = 0;

                do
                {
                    dwSize = 0;
                    if (!WinHttpQueryDataAvailable(hRequest, &dwSize) || dwSize == 0) break;

                    size_t oldSize = out.size();
                    out.resize(oldSize + dwSize);

                    if (!WinHttpReadData(hRequest, &out[oldSize], dwSize, &dwDownloaded))
                    {
                        out.resize(oldSize);
                        break;
                    }

                    if (dwDownloaded < dwSize)
                        out.resize(oldSize + dwDownloaded);

                } while (dwSize > 0);
            }
        }
        WinHttpCloseHandle(hRequest);
    }
}

void hijack_requestor()
{
    requestor_log("[0x%p] hijack_requestor installing raw thunk hooks\n", NtCurrentThreadId());
    
    hSession = WinHttpOpen(L"NLR/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (hSession)
        hConnection = WinHttpConnect(hSession, L"127.0.0.1", 30031, 0); 
    
    requestor_log("[0x%p] raw hSession=0x%p hConnection=0x%p\n", NtCurrentThreadId(), hSession, hConnection);

    // Исправлено: добавлено приведение к (void*) и 5 аргументов для HookFn
    HookFn((void*)0x41BC78E0, (void*)GetSerial, 0, NULL, 0);
    HookFn((void*)0x41BC98E0, (void*)MakeRequest, 0, NULL, 0);

    requestor_log("[0x%p] hijack_requestor installed raw thunk hooks\n", NtCurrentThreadId());
}

#else // !GHETTO_FIX

// Реализация через наследование класса Requestor (если GHETTO_FIX не определен)

void* reqtram = nullptr;

static void requestor_log(const char* fmt, ...) { /* ... аналогично ... */ }

void* hkReqInst()
{
    printf("[0x%p] 0x%p Requestor::Instance\n", NtCurrentThreadId(), _ReturnAddress());
    // Вызов оригинала через трамплин
    return reinterpret_cast<void*(*)()>(reqtram)();
}

class NLR_Requestor : public neverlosesdk::network::Requestor
{
    HINTERNET hSession;
    HINTERNET hConnection;

    static std::string with_token(std::string_view route)
    {
        std::string resolved(route);
        ensure_auth_token_loaded();
        if (!auth_token || !auth_token[0]) return resolved;
        if (resolved.find("token=") != std::string::npos) return resolved;

        if (!resolved.empty() && resolved[0] == '/')
            resolved += (resolved.find('?') == std::string::npos) ? '?' : '&';
        else
            return resolved;

        resolved += "token=";
        resolved += auth_token;
        return resolved;
    }

    void MakeRequest(std::string& out, std::string_view route, int _, int __) override
    {
        const std::string resolved_route = with_token(route);
        new (&out) std::string("");

        int size_needed = MultiByteToWideChar(CP_UTF8, 0, resolved_route.data(), (int)resolved_route.size(), NULL, 0);
        wchar_t* wroute = (wchar_t*)malloc((size_needed + 1) * sizeof(wchar_t));

        if (wroute)
        {
            MultiByteToWideChar(CP_UTF8, 0, resolved_route.data(), (int)resolved_route.size(), wroute, size_needed);
            wroute[size_needed] = L'\0';
            HINTERNET hRequest = WinHttpOpenRequest(hConnection, L"GET", wroute, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
            free(wroute);
            if (hRequest && WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0))
            {
                if (WinHttpReceiveResponse(hRequest, NULL))
                {
                    DWORD dwSize = 0;
                    DWORD dwDownloaded = 0;
                    do {
                        dwSize = 0;
                        if (!WinHttpQueryDataAvailable(hRequest, &dwSize) || dwSize == 0) break;
                        size_t oldSize = out.size();
                        out.resize(oldSize + dwSize);
                        if (!WinHttpReadData(hRequest, &out[oldSize], dwSize, &dwDownloaded)) {
                            out.resize(oldSize);
                            break;
                        }
                    } while (dwSize > 0);
                }
            }
            WinHttpCloseHandle(hRequest);
        }
    }

    void GetSerial(std::string& out, nlohmann::json& request) override
    {
        new (&out) std::string("g6w/cgN2AuDsLw3xrzboM1kbkLy+osvg0Y/j0LJnQf04GHbV8s5V4yReEk1mh3ZA2G72fHG3oOh7zlGEfR1nKw717WiwRwsrgSDfJtaTQz14VDDkayLBNV1DaT/qSyx8Frg1nXU0crRu1P/G+EPvH6nWNPYLZdUMIeqVCToEFhJnqiuRoAyypjFNiKnLEMiy5j2YvBcLCOC8yC3FPt/GGsvUldBqkmQGkBjIsXsSkut05txVxq7VDx1i9adKE4zalTzNHr0Vtd6DTr8aeH8NYHWPGWAsnTBkZlkNuRuhBTtgRTcIKxzGATTN4k8/JaXCpxri7IqsylvZgXQw+5zldLjAHqcAWw3OD5iQn8DtOoon+DrHm3k3FY6wIrCM1FzTdjAIcTvXSiWOURHiwA4sJ8ExR4dyBZMydo8aBAYjrRxcD9oDa/VVJT4cZfDkyWvRjI3WMyEajF2JhiGcjpjztmD8fyt9C16VXwLfoYuJnrX1/Dv8SZfCU6U2UhwJlxO5mkg+/IctveCdxy8IIiXTKwA5vmiEpXRuUu17SCdmJhFLZ+Jr6cTmrob4exSEggGRk6BTaVomOq4I6IpkVUBIUVup+4JvWFseL5UkPOQqHIO5Rxnj1jY+PjAWFPeeXSZsP8/ceEnX8J13tfb7PAqRSrpQ1Wv/y+OjaqMoPg9PiRE=");
    }

    void fn2() override {}

    void fn3(std::string& out, nlohmann::json& request) override
    {
        new (&out) std::string("{}");
    }

    void QueryLuaLibrary(std::string& out, std::string_view name) override
    {
        new (&out) std::string(name);
    }

public:
    NLR_Requestor()
    {
        hSession = WinHttpOpen(L"NLR/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
        if (hSession)
            hConnection = WinHttpConnect(hSession, L"127.0.0.1", 30031, 0);
    }
};

void hijack_requestor()
{
    // Подмена глобального указателя на Requestor
    *(neverlosesdk::network::Requestor**)0x42518C58 = new NLR_Requestor;
    *(PDWORD)0x42518C54 = 0x80000004;

    // Исправлено: 5 аргументов для HookFn и использование &reqtram
    HookFn((void*)0x41BC9450, (void*)hkReqInst, 0, &reqtram, 0);
}

#endif // GHETTO_FIX