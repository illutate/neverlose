#ifndef NEVERLOSE_LOGGER_H
#define NEVERLOSE_LOGGER_H
#include <ctime>
#include <string>
#include <iostream>
#include <iomanip>

class clogger {
    std::wostream& stream;
    std::wstring name;
public:
    clogger(std::wstring section_name, std::wostream& stream) 
        : name(std::move(section_name)), stream(stream) 
    {
        stream << L"[BEGIN] " << name << L"\n";
    }

    ~clogger() {
        stream << L"[ END ] " << name << L"\n";
        stream.flush();
    }

    // Шаблон для типов, которые wostream уже знает (int, float, wchar_t*)
    template<typename T>
    clogger& operator<<(const T& in) {
        stream << in;
        return *this;
    }

    // ИСПРАВЛЕНИЕ C2297: Перегрузка для обычных строк (char*)
    clogger& operator<<(const char* in) {
        if (in) {
            while (*in) stream << (wchar_t)*in++;
        }
        return *this;
    }

    // ИСПРАВЛЕНИЕ: Перегрузка для PVOID (void*), чтобы выводить адреса как HEX
    clogger& operator<<(void* in) {
        stream << std::hex << std::showbase << (uintptr_t)in << std::dec << std::noshowbase;
        return *this;
    }
};


#define ENTER_LOGGER(log_manager_obj) log_manager_obj.section(__FUNCTIONW__)

class clog_manager
{
    std::wostream& stream;
    struct null_buffer : std::wstreambuf {
        wint_t overflow(wint_t c) override { return c; }
    };
    static inline null_buffer nb;
    static inline std::wostream nullb = std::wostream(&nb);

public:
    clog_manager() : stream(nullb) {};
    explicit clog_manager(std::wostream& stream) : stream(stream)
    {
        std::time_t now = std::time(nullptr);
        tm newtime;
        localtime_s(&newtime, &now);
        stream << L"Logger inited at " << std::put_time(&newtime, L"%Y-%m-%d %H:%M:%S") << L"\n";
    };

    ~clog_manager()
    {
        stream.flush();
    };

    clogger section(const wchar_t* s_name) {
        return clogger(s_name, stream);
    }
};

#endif // NEVERLOSE_LOGGER_H