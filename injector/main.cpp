#include <Windows.h>
#include <TlHelp32.h>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <thread>
#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>

namespace
{
    std::string g_dll_name;
    std::string g_server_exe;
    constexpr const char* kConfigPath = "config.ini";
    constexpr const char* kServerConfigPath = "server_config.ini";
    constexpr const char* kWindowClass = "Valve001";
    constexpr DWORD kProcessAccess = PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
        PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ;

    void set_color(int color)
    {
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
    }

    void sleep_human()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    void print_centered(const char* text, bool delay = true)
    {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
        {
            int width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
            int len = static_cast<int>(strlen(text));
            // Учитываем, что UTF-8 символы (✨) могут занимать больше байт, но в консоли выглядят как один символ
            int padding = (width - len) / 2;
            if (padding > 0)
            {
                for (int i = 0; i < padding; ++i) std::printf(" ");
            }
        }
        std::printf("%s\n", text);
        if (delay) sleep_human();
    }

    void clear_console()
    {
        system("cls");
    }

    void kill_process_by_name(const char* name)
    {
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snap == INVALID_HANDLE_VALUE) return;

        PROCESSENTRY32W pc = { sizeof(pc) };
        std::string s_name(name);
        std::wstring w_name(s_name.begin(), s_name.end());

        if (Process32FirstW(snap, &pc))
        {
            do {
                if (!_wcsicmp(pc.szExeFile, w_name.c_str()))
                {
                    HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, pc.th32ProcessID);
                    if (hProc)
                    {
                        TerminateProcess(hProc, 0);
                        CloseHandle(hProc);
                    }
                }
            } while (Process32NextW(snap, &pc));
        }
        CloseHandle(snap);
    }

    void select_files()
    {
        std::ifstream dll_in(kConfigPath);
        if (dll_in.good()) std::getline(dll_in, g_dll_name);
        dll_in.close();

        if (g_dll_name.empty() || !std::filesystem::exists(g_dll_name))
        {
            set_color(15);
            std::printf("Enter DLL filename (e.g., neverlose.dll): ");
            std::cin >> g_dll_name;
            std::ofstream dll_out(kConfigPath);
            dll_out << g_dll_name;
            dll_out.close();
        }

        std::ifstream srv_in(kServerConfigPath);
        if (srv_in.good()) std::getline(srv_in, g_server_exe);
        srv_in.close();

        if (g_server_exe.empty() || !std::filesystem::exists(g_server_exe))
        {
            set_color(15);
            std::printf("Enter Local Server filename (e.g., neverlose-server.exe): ");
            std::cin >> g_server_exe;
            std::ofstream srv_out(kServerConfigPath);
            srv_out << g_server_exe;
            srv_out.close();
        }
        clear_console();
    }

    HANDLE start_server_silent()
    {
        STARTUPINFOA si = { sizeof(si) };
        PROCESS_INFORMATION pi;
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;

        if (CreateProcessA(nullptr, const_cast<char*>(g_server_exe.c_str()), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi))
        {
            CloseHandle(pi.hThread);
            return pi.hProcess;
        }
        return nullptr;
    }

    void print_banner()
    {
        SetConsoleOutputCP(CP_UTF8); // Для корректного отображения ✨
        SetConsoleTitleA("@project_320 neverlose injector library");
        set_color(15);
        print_centered("bob x spiny fixed...", false);
        print_centered(">.< neverlose injector ✨", false);
        print_centered("@project_320 compiled x released", false);
        std::puts("");
    }

    LPVOID GetModBase(DWORD pid, const wchar_t* name)
    {
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
        if (snap == INVALID_HANDLE_VALUE) return nullptr;
        MODULEENTRY32W me = { sizeof(me) };
        LPVOID base = nullptr;
        for (BOOL ok = Module32FirstW(snap, &me); ok; ok = Module32NextW(snap, &me))
        {
            if (!_wcsicmp(me.szModule, name))
            {
                base = me.modBaseAddr;
                break;
            }
        }
        CloseHandle(snap);
        return base;
    }

    void RestoreNtOpenFile(HANDLE hProcess)
    {
        HMODULE hNtdll = GetModuleHandleW(L"ntdll");
        LPVOID pLocal = GetProcAddress(hNtdll, "NtOpenFile");
        if (!pLocal) return;
        DWORD pid = GetProcessId(hProcess);
        LPVOID pRemote = GetModBase(pid, L"ntdll.dll");
        if (!pRemote) return;
        LPVOID target = (LPVOID)((uintptr_t)pRemote + ((uintptr_t)pLocal - (uintptr_t)hNtdll));
        char orig[5] = { 0 };
        wchar_t path[MAX_PATH];
        GetSystemDirectoryW(path, MAX_PATH);
        wcscat_s(path, L"\\ntdll.dll");
        HMODULE hFresh = LoadLibraryExW(path, nullptr, DONT_RESOLVE_DLL_REFERENCES);
        if (hFresh)
        {
            LPVOID pFn = GetProcAddress(hFresh, "NtOpenFile");
            if (pFn) memcpy(orig, pFn, 5);
            FreeLibrary(hFresh);
        }
        if (!*(DWORD*)orig) return;
        DWORD oldProt;
        if (VirtualProtectEx(hProcess, target, 5, PAGE_EXECUTE_READWRITE, &oldProt))
        {
            WriteProcessMemory(hProcess, target, orig, 5, nullptr);
            VirtualProtectEx(hProcess, target, 5, oldProt, &oldProt);
        }
    }
}

int main()
{
    select_files();
    print_banner();

    set_color(10);
    if (std::filesystem::exists(g_dll_name))
        print_centered(("[ d l l ] found " + g_dll_name).c_str());
    else {
        print_centered(("[ e r r o r ] failed to find " + g_dll_name).c_str());
        Sleep(3000); return 1;
    }

    if (std::filesystem::exists(g_server_exe))
        print_centered(("[ d e b u g ] found " + g_server_exe).c_str());
    else {
        print_centered(("[ e r r o r ] failed to find " + g_server_exe).c_str());
        Sleep(3000); return 1;
    }

    print_centered("[ d e b u g ] starting local server hosting...");
    HANDLE hServer = start_server_silent();

    print_centered("[ d e b u g ] waiting for csgo.exe...");
    
    DWORD process_id = 0;
    HWND window = nullptr;
    
    // Цикл ожидания окна с проверкой закрытия самого инжектора
    while (!window)
    {
        window = FindWindowA(kWindowClass, nullptr);
        if (!window)
        {
            // Если консоль закрывается пользователем, Windows пошлет сигнал, 
            // но для надежности: если main() прерывается, деструкторы не сработают.
            // В обычном режиме просто ждем.
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        GetWindowThreadProcessId(window, &process_id);
    }

    print_centered("[ d l l ] found csgo.exe");

    char full_dll_path[MAX_PATH]{};
    GetFullPathNameA(g_dll_name.c_str(), MAX_PATH, full_dll_path, nullptr);

    HANDLE process = OpenProcess(kProcessAccess, FALSE, process_id);
    if (!process || process == INVALID_HANDLE_VALUE)
    {
        print_centered("[ e r r o r ] failed to open process. RUN AS ADMINISTRATOR");
        if (hServer) TerminateProcess(hServer, 0);
        Sleep(3000); return 1;
    }

    RestoreNtOpenFile(process);
    const SIZE_T path_length = std::strlen(full_dll_path) + 1;
    LPVOID remote_path = VirtualAllocEx(process, nullptr, path_length, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    
    if (remote_path)
    {
        WriteProcessMemory(process, remote_path, full_dll_path, path_length, nullptr);
        FARPROC load_library = GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
        HANDLE remote_thread = CreateRemoteThread(process, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(load_library), remote_path, 0, nullptr);

        if (remote_thread)
        {
            WaitForSingleObject(remote_thread, INFINITE);
            set_color(13);
            print_centered("[ d e b u g ] dll injected successfully...");
            CloseHandle(remote_thread);
        }
        VirtualFreeEx(process, remote_path, 0, MEM_RELEASE);
    }

    ShowWindow(GetConsoleWindow(), SW_HIDE);

    // Ожидаем закрытия CS:GO
    while (true)
    {
        DWORD exitCode = 0;
        if (!GetExitCodeProcess(process, &exitCode) || exitCode != STILL_ACTIVE)
            break;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    kill_process_by_name(g_server_exe.c_str());
    if (hServer) {
        TerminateProcess(hServer, 0);
        CloseHandle(hServer);
    }
    CloseHandle(process);
    return 0;
}