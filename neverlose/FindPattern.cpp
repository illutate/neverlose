#include "FindPattern.h"

// @project_320: debug log - генерация таблицы смещений Хорспула для ускорения поиска
void generate_shift_table(BYTE out[256], const PBYTE pattern, size_t pattern_len, BYTE wild_card)
{
    // @project_320: debug log - расчет максимального безопасного прыжка
    size_t last_wildcard_pos = pattern_len;
    for (size_t i = pattern_len; i > 0; i--)
    {
        if (pattern[i - 1] == wild_card) 
        {
            last_wildcard_pos = i - 1;
            break;
        }
    }

    size_t max_shift = pattern_len - last_wildcard_pos;
    if (max_shift == 0 || last_wildcard_pos == pattern_len)
        max_shift = 1;

    // @project_320: debug log - заполнение таблицы дефолтным сдвигом
    memset(out, (BYTE)max_shift, 256);

    // @project_320: debug log - уточнение сдвигов для символов паттерна (исключая последний)
    if (pattern_len > 1) 
    {
        for (size_t j = 0; j < pattern_len - 1; j++)
        {
            if (pattern[j] != wild_card)
                out[pattern[j]] = (BYTE)(pattern_len - 1 - j);
        }
    }
};

// @project_320: debug log - основной поиск сигнатуры в памяти csgo.exe
void* FindPattern(void* base, size_t scan_size, const PBYTE pattern, size_t pattern_len, BYTE wild_card, size_t offset)
{
    // @project_320: debug log - защита: паттерн не может быть пустым или больше области поиска
    if (!base || !pattern || pattern_len == 0 || scan_size < pattern_len)
        return nullptr;

    BYTE shift_table[256];
    
    // @project_320: debug log - SEH защита на случай сканирования защищенных страниц (Guard Pages)
    __try 
    {
        generate_shift_table(shift_table, pattern, pattern_len, wild_card);

        PBYTE cursor = (PBYTE)base;
        PBYTE bound = cursor + scan_size - pattern_len;

        while (cursor <= bound)
        {
            // @project_320: debug log - сравнение с конца паттерна (классический Horspool)
            for (size_t i = pattern_len - 1; ; i--)
            {
                if (pattern[i] != wild_card && cursor[i] != pattern[i])
                {
                    // @project_320: debug log - прыжок по таблице смещений
                    cursor += shift_table[cursor[pattern_len - 1]];
                    break;
                }

                if (i == 0) // @project_320: debug log - паттерн найден
                    return (void*)(cursor + offset);
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        // @project_320: debug log - критическая ошибка: доступ к нечитаемой памяти во время поиска
        return nullptr;
    }

    return nullptr;
};