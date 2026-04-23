#ifndef NEVELOSE_CSGO_MINI_H
#define NEVELOSE_CSGO_MINI_H
#include <phnt_windows.h>
#include <phnt.h>

// @project_320: debug log - тип функции для создания интерфейсов движка Source
typedef void* (*CreateInterfaceFn)(const char* pName, int* pReturnCode);

// @project_320: debug log - базовый класс для всех систем движка с защитой виртуальной таблицы
class IAppSystem
{
public:
    virtual bool Connect(CreateInterfaceFn factory) = 0;
    virtual void Disconnect() = 0;

    virtual void* QueryInterface(const char* pInterfaceName) = 0;

    virtual DWORD Init() = 0;
    virtual void Shutdown() = 0;

    virtual const PVOID GetDependencies() = 0;

    virtual DWORD GetTier() = 0;

    virtual void Reconnect(CreateInterfaceFn factory, const char* pInterfaceName) {}
    virtual bool IsSingleton() { return true; }
};

// @project_320: debug log - интерфейс управления консольными переменными и командами
class ICvar : public IAppSystem
{
public:
    virtual DWORD AllocateDLLIdentifier() = 0;

    virtual void            RegisterConCommand(PVOID pCommandBase) = 0;
    virtual void            UnregisterConCommand(PVOID pCommandBase) = 0;
    virtual void            UnregisterConCommands(DWORD id) = 0;

    virtual const char* GetCommandLineValue(const char* pVariableName) = 0;

    virtual PVOID FindCommandBase(const char* name) = 0;
    virtual const PVOID FindCommandBase(const char* name) const = 0;
    virtual PVOID FindVar(const char* var_name) = 0;
    virtual const PVOID FindVar(const char* var_name) const = 0;
};

// @project_320: debug log - обертка для безопасного поиска переменных без риска краша
inline PVOID SafeFindVar(ICvar* cvar_interface, const char* var_name)
{
    // @project_320: debug log - жесткая проверка указателя на интерфейс cvar
    if (!cvar_interface || !var_name)
        return nullptr;

    __try
    {
        // @project_320: debug log - попытка вызова оригинального метода FindVar игры
        return cvar_interface->FindVar(var_name);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        // @project_320: debug log - перехвачен краш: интерфейс cvar невалиден или память защищена
        return nullptr;
    }
}

// @project_320: debug log - макрос для безопасного получения интерфейса
#define GET_INTERFACE(factory, name) \
    [&]() -> void* { \
        __try { return factory(name, nullptr); } \
        __except(EXCEPTION_EXECUTE_HANDLER) { return nullptr; } \
    }()

#endif // NEVELOSE_CSGO_MINI_H