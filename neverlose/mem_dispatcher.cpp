#include "internal_fixes.h"
#include "HookFn.h"
#include "FindPattern.h"
#include <cstdio>
#include "detours.h"
#include <vector>

// Добавляем объявление пространства имен, если оно не подтянулось из internal_fixes.h
namespace neverlose {
    void fix_mem_dispatcher();
}

enum operation_t
{
    OPERATION_REGISTER_HOOK = 1,
    OPERATION_EMPLACE_HOOKS,
    OPERATION_ERASE_HOOKS,
    OPERATION_SIGSCAN = 6,
};

#pragma pack(push, 1)
struct sigscan_t
{
    PVOID64 Base;
    PVOID64 Signature;
    size_t Length;
    PVOID64 Result;
};

struct hook_t
{
    PVOID64 Address;
    PVOID64 Hook;
    PVOID64 pTrampoline;
};
#pragma pack(pop)

struct HookDesc
{
    bool IsActive;
    PVOID Address;
    PVOID Trampoline;
    PVOID Hook;
};

// Исправлено: инициализация ссылки на вектор
static auto& g_HkDesc = *reinterpret_cast<std::vector<HookDesc>*>(0x42500C44);
static bool TransactionAlive = false;

// Исправленная функция диспетчера
BOOL __cdecl hkMemDispatcher(operation_t type, void* ptr)
{
    BOOL result = FALSE;

    switch (type)
    {
    case OPERATION_SIGSCAN:
    {
        auto* data = (sigscan_t*)ptr;
        // Исправлено: FindPattern требует 6 аргументов согласно FindPattern.h
        // Добавлен 6-й аргумент: offset = 0
        data->Result = (PVOID64)FindPattern((void*)data->Base, 0x10000000, (PBYTE)data->Signature, data->Length, 0xCC, 0);
        result = (data->Result != nullptr);
    }
    break;

    case OPERATION_REGISTER_HOOK:
    {
        if (!TransactionAlive)
        {
            DetourTransactionBegin();
            DetourUpdateThread(GetCurrentThread());
            TransactionAlive = true;
        }

        auto* data = (hook_t*)ptr;
        PVOID pTramp = (PVOID)data->Address;
        
        // Пропускаем специфический адрес engine.dll, если нужно
        if (data->Address == (PVOID64)((PBYTE)GetModuleHandle(L"engine.dll") + 0xF0470)) return TRUE;

        if (DetourAttachEx(&pTramp, (PVOID)data->Hook, (PDETOUR_TRAMPOLINE*)data->pTrampoline, NULL, NULL) == NO_ERROR)
        {
            result = TRUE;
        }
        else
            result = FALSE;
    }
    break;

    case OPERATION_EMPLACE_HOOKS:
        if (TransactionAlive)
        {
            DetourTransactionCommit();
            TransactionAlive = false;
            result = TRUE;
        }
        else
            result = FALSE;
        break;

    case OPERATION_ERASE_HOOKS:
    {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

        for (auto& hook : g_HkDesc)
        {
            if (hook.IsActive && hook.Trampoline)
            {
                DetourDetach(&hook.Trampoline, hook.Hook);
                hook.IsActive = false;
            }
        }
        DetourTransactionCommit();
        result = TRUE;
    }
    break;
    }

    return result;
}

// Исправлено: реализация функции внутри пространства имен, чтобы избежать C2653
namespace neverlose {
    void fix_mem_dispatcher_install()
    {
        // Вызов HookFn с 5 аргументами согласно HookFn.h
        HookFn((void*)0x41B6C450, (void*)hkMemDispatcher, 0, NULL, 0);
    }
}