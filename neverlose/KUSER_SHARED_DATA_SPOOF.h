#ifndef NEVERLOSE_KUSER_SHARED_DATA_SPOOF_H
#define NEVERLOSE_KUSER_SHARED_DATA_SPOOF_H

#pragma once
#include <windows.h>
#include <array>
#include <vector>
#include "REGS.h"

// Структура для аллокатора (трамплин)
struct kuser_data_spoof
{
    REG reg;
    PVOID fake_kuser;
    uintptr_t JumpBackAddr;
    unsigned char data[128]; // Буфер под машинный код патча

    // Конструктор для ArenaAllocator::construct
    kuser_data_spoof(REG r, PVOID fake) : reg(r), fake_kuser(fake), JumpBackAddr(0) {
        memset(data, 0, sizeof(data));
    }
};

struct kuser_data_spoof_info
{
    uintptr_t address;
    REG reg;
    size_t nops;
};

// Используем std::vector или обычный массив для совместимости
inline const std::vector<kuser_data_spoof_info> g_kuser_spoofs = 
{
    { 0x42E1614F, REG::EAX, 2 },
    { 0x4292094C, REG::EAX, 3 },
    { 0x4279B4B0, REG::EAX, 2 },
    { 0x4282E5B6, REG::EAX, 6 },
    { 0x4278815E, REG::ECX, 2 },
    { 0x42B15516, REG::ECX, 3 },
    { 0x434C48EF, REG::ECX, 2 },
    { 0x42AB63D3, REG::EDX, 2 },
    { 0x431CD6B3, REG::EDX, 2 },
};

// Константы байт-кода
inline const BYTE DEF_KUSER_SPOOF[] =
{
    // cmp reg, 7FFE0000h
    0x81, 0xCC, 0x00, 0x00, 0xFE, 0x7F,
    // jb +18h
    0x0F, 0x82, 0x18, 0x00, 0x00, 0x00,
    // cmp reg, 7FFE1000h
    0x81, 0xCC, 0x00, 0x10, 0xFE, 0x7F,
    // ja +0Ch
    0x0F, 0x87, 0x0C, 0x00, 0x00, 0x00,
    // sub reg, 7FFE0000h
    0x81, 0xCC, 0x00, 0x00, 0xFE, 0x7F,
    // add reg, spoof
    0x81, 0xCC, 0x00, 0x00, 0x00, 0x00 
};

#endif // NEVERLOSE_KUSER_SHARED_DATA_SPOOF_H