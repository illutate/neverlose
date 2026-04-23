#ifndef NEVERLOSE_HOOKFN_H
#define NEVERLOSE_HOOKFN_H

#include <phnt_windows.h>
#include <phnt.h>


// Макрос для получения стандартного адреса возврата (после JMP)
#define GET_DEF_TRAM(addr) ((unsigned char*)(addr) + 5)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * HookFn - устанавливает JMP-хук (0xE9)
 * * @param Dst         - Адрес функции, которую перехватываем
 * @param Src         - Адрес нашей функции-обработчика
 * @param NopBytes    - Кол-во NOP-ов после основного прыжка
 * @param TrampOut    - [Out] Указатель на выделенный трамплин для вызова оригинала
 * @param TrampOffset - Смещение внутри Dst для формирования трамплина
 */
NTSTATUS NTAPI HookFn(
    void* Dst, 
    void* Src, 
    SIZE_T NopBytes, 
    void** TrampOut, 
    size_t TrampOffset
);

#ifdef __cplusplus
}
#endif

// Для C++ удобства делаем inline-обертку с аргументами по умолчанию ВНЕ extern "C"
#ifdef __cplusplus
inline NTSTATUS HookFnCpp(void* Dst, void* Src, SIZE_T NopBytes, void** TrampOut = nullptr, size_t TrampOffset = 0) {
    return HookFn(Dst, Src, NopBytes, TrampOut, TrampOffset);
}
#endif

#endif // NEVERLOSE_HOOKFN_H