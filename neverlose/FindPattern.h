#ifndef NEVERLOSE_FIND_PATTERN_H
#define NEVERLOSE_FIND_PATTERN_H
#include <phnt_windows.h>
#include <phnt.h>

// @project_320: debug log - макрос для удобного объявления паттернов. 
// Автоматически вычисляет размер, исключая завершающий нуль-терминатор.
#define PATTERN(signature) (PBYTE)signature, (size_t)(sizeof(signature) - 1)

// @project_320: debug log - предварительный расчет таблицы смещений (алгоритм Horspool)
// out: таблица байтов [256], pattern: искомая последовательность, wild_card: байт-маска (обычно 0xCC или '?')
void generate_shift_table(BYTE out[256], const PBYTE pattern, size_t pattern_len, BYTE wild_card);

/**
 * @project_320: debug log - функция поиска сигнатуры в адресном пространстве csgo.exe
 * @param base Начальный адрес поиска (обычно база модуля .dll)
 * @param scan_size Размер области сканирования (SizeOfImage)
 * @param pattern Указатель на массив байт сигнатуры
 * @param pattern_len Длина сигнатуры
 * @param wild_card Байт, который считается "любым" (маска)
 * @param offset Смещение, которое будет добавлено к найденному адресу
 */
void* FindPattern(void* base, size_t scan_size, const PBYTE pattern, size_t pattern_len, BYTE wild_card, size_t offset);

// @project_320: debug log - встроенная обертка для поиска в модуле целиком без ручного замера размера
template <typename T = void*>
inline T FindPatternInRange(uintptr_t start, uintptr_t end, const PBYTE pattern, size_t len, BYTE mask = 0xCC, size_t offset = 0) {
    if (start >= end) return nullptr;
    return (T)FindPattern((void*)start, (size_t)(end - start), pattern, len, mask, offset);
}

#endif // NEVERLOSE_FIND_PATTERN_H