#ifndef NEVERLOSE_NEVERLOSE_H
#define NEVERLOSE_NEVERLOSE_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define PHNT_VERSION PHNT_THRESHOLD // Самая совместимая версия

#include <phnt_windows.h>
#include <phnt.h>

// #include <winternl.h> // УДАЛИ ИЛИ ЗАКОММЕНТИРУЙ

// 4. Твои остальные заголовки
#include "logger.h"
#include "resource.h"
#include <array>
#include <vector>

// Упреждающие объявления (Forward declarations), чтобы не зависеть от порядка файлов
class clog_manager; 

class neverlose
{
	// Приватные методы для внутренней работы
	PVOID load_res_to_mem(int idr, const char* rcname) const;
	
	// panic теперь принимает переменное число аргументов (fmt)
	[[noreturn]] static void panic(const char* fmt, ...);

	neverlose(const neverlose&) = delete;
	neverlose(neverlose&&) = delete;
	neverlose& operator=(const neverlose&) = delete;

	void fix_imports();
	void spoof_peb();
	void spoof_cpuid();
	void spoof_kusd();
	void fix_interfaces();
	void fix_cvars();
	void fix_signatures();

public:
	// Конструктор инициализирует базовые параметры маппинга
	neverlose() : baseAddr((PVOID)0x412A0000), imageSize(0x3501000), hThis(nullptr) {};

	void map(HMODULE hThis);
	void fix_dump();
	void spoof();
	void setup_hooks();
	void entry();
	void set_veh();

	// Проверка: находится ли адрес внутри нашего замапленного модуля
	bool in_range(void* addr) const { 
		return addr >= baseAddr && addr < ((char*)baseAddr + imageSize); 
	}

	void* base() const { return baseAddr; }

private:
	PVOID baseAddr;
	SIZE_T imageSize;
	clog_manager logman; // Твой менеджер логов
	HINSTANCE hThis;
};

// Глобальный объект чита, который мы используем в main.cpp
extern neverlose g_neverlose;

#endif // NEVERLOSE_NEVERLOSE_H