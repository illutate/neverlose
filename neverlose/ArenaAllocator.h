#ifndef NEVERLOSE_ARENA_ALLOCATOR_H
#define NEVERLOSE_ARENA_ALLOCATOR_H

#include <phnt_windows.h>
#include <phnt.h>
#include <utility>
#include <new> // Обязательно для placement new

// Макрос для текущего процесса, если его нет в заголовках
#ifndef NtCurrentProcess
#define NtCurrentProcess() ((HANDLE)(LONG_PTR)-1)
#endif

template<typename T>
class ArenaAllocator
{
    using type = T;
    using pointer = T*;
    using size_type = SIZE_T;

    size_type capacity;
    size_type cursor;
    size_type allocation_size;
    PVOID arena;

public:
    ArenaAllocator(size_t initial_capacity) : capacity(initial_capacity), cursor(0), allocation_size(0), arena(nullptr)
    {
        if (initial_capacity == 0) return;

        SIZE_T Size = initial_capacity * sizeof(type);
        PVOID Addr = NULL;

        // Прямой вызов из ntdll.dll (предполагается, что проект линкуется с ntdll.lib или имеет прототипы)
        // Если NtAllocateVirtualMemory не виден, используйте VirtualAlloc
        NTSTATUS status = NtAllocateVirtualMemory(NtCurrentProcess(), &Addr, 0, &Size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        
        if (status >= 0) // Аналог NT_SUCCESS
        {
            arena = Addr;
            allocation_size = Size;
        }
    }

    ~ArenaAllocator() 
    {
        // В деструкторе желательно освобождать память, чтобы не было утечек при перезагрузке чита
        if (arena) {
            SIZE_T Size = 0;
            NtFreeVirtualMemory(NtCurrentProcess(), &arena, &Size, MEM_RELEASE);
        }
    }

    bool has_scene() const { return arena != nullptr; }

    template<typename ...Args>
    pointer construct(Args&&... args)
    {
        if (!arena || cursor >= capacity) 
        {
            return nullptr; 
        }

        __try 
        {
            // Вычисляем адрес: приводим arena к указателю на тип T
            pointer cobj = &((pointer)arena)[cursor];
            
            // Инкремент курсора
            cursor++;

            // Placement new: создаем объект в уже выделенной памяти
            return new (cobj) type(std::forward<Args>(args)...);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) 
        {
            return nullptr;
        }
    }

    void reset() 
    {
        cursor = 0;
    }

    size_type get_cursor() const { return cursor; }
};

#endif // NEVERLOSE_ARENA_ALLOCATOR_H