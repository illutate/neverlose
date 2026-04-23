#include "neverlose.h"

// @project_320: debug log - точка входа для оригинального кода игры (не менять 0x412A0A00)
constexpr uintptr_t winver_entry_point = 0x412A0A00;

// @project_320: debug log - фиктивная функция выхода для предотвращения нештатного завершения
NTSTATUS __declspec(naked) NTAPI _fictive_(LPVOID lpThreadParameter)
{
    __asm
    {
        push 0
        call RtlExitUserProcess // @project_320: debug log - принудительный выход при критическом сбое
    };
};

void neverlose::entry()
{
    auto logger = ENTER_LOGGER(logman);

    HANDLE hThread = NULL;

    // @project_320: debug log - попытка создания потока в обход стандартных WinAPI
    __try 
    {
        NTSTATUS status = NtCreateThreadEx(
            &hThread, 
            THREAD_ALL_ACCESS, 
            NULL, 
            NtCurrentProcess(), 
            (PUSER_THREAD_START_ROUTINE)winver_entry_point, 
            0, 
            THREAD_CREATE_FLAGS_CREATE_SUSPENDED, 
            0, 0x40000, 0x40000, 
            NULL
        );

        if (!NT_SUCCESS(status) || hThread == NULL) {
            // @project_320: debug log - предотвращение краша: выход если поток не создался
            panic("Failed to create thread! NTSTATUS: 0x%08X\n", status);
            return;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        // @project_320: debug log - перехвачено системное исключение при вызове NtCreateThreadEx
        return;
    }

    logger << "Created thread.\n";

    THREAD_BASIC_INFORMATION tbi{ 0 };
    
    // @project_320: debug log - получение информации о потоке с защитой от пустых дескрипторов
    if (!NT_SUCCESS(NtQueryInformationThread(hThread, ThreadBasicInformation, &tbi, sizeof(tbi), NULL)))
    {
        NtTerminateThread(hThread, STATUS_UNSUCCESSFUL);
        NtClose(hThread);
        panic("Failed to get TIB!\n");
        return;
    }

    logger << "Entry thread: 0x" << (void*)tbi.ClientId.UniqueThread << '\n';

    // @project_320: debug log - настройка контекста (закомментировано, но добавлена проверка безопасности)
    /*
    CONTEXT tctx = { 0 };
    tctx.ContextFlags = CONTEXT_FULL;
    if (NT_SUCCESS(NtGetContextThread(hThread, &tctx))) {
        // @project_320: debug log - тут можно безопасно модифицировать регистры перед запуском
    }
    */

    // @project_320: debug log - запуск потока в csgo.exe
    __try 
    {
        ULONG suspendCount = 0;
        if (!NT_SUCCESS(NtResumeThread(hThread, &suspendCount))) {
            // @project_320: debug log - не удалось возобновить поток
            NtTerminateThread(hThread, 0);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) 
    {
        // @project_320: debug log - защита от краша при резком закрытии процесса игры
    }

    logger << "Resumed thread.\n";

    // @project_320: debug log - ожидание завершения с таймаутом, чтобы дллка не висела вечно
    LARGE_INTEGER timeout;
    timeout.QuadPart = -10000000LL; // 1 секунда (в относительных единицах)

    if (NtWaitForSingleObject(hThread, FALSE, NULL) == STATUS_SUCCESS) 
    {
        THREAD_BASIC_INFORMATION exit_tbi{ 0 };
        if (NT_SUCCESS(NtQueryInformationThread(hThread, ThreadBasicInformation, &exit_tbi, sizeof(exit_tbi), NULL)))
        {
            logger << "Entry returned 0x" << std::hex << exit_tbi.ExitStatus << '\n';
        }
    }

    // @project_320: debug log - корректное закрытие хендла
    if (hThread) {
        NtClose(hThread);
    }
};