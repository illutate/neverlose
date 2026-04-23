#pragma once
#ifndef NEVERLOSE_TOKEN_H
#define NEVERLOSE_TOKEN_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>

// Используем inline, чтобы избежать ошибок линковки LNK2005 при множественном включении
inline std::string g_auth_token_storage;
inline const char* auth_token = "";

// Получаем адрес текущего модуля
extern "C" IMAGE_DOS_HEADER __ImageBase;

namespace token_detail
{
    inline std::string trim(const std::string& s)
    {
        auto first = std::find_if_not(s.begin(), s.end(), [](unsigned char ch) { return std::isspace(ch); });
        if (first == s.end()) return "";
        auto last = std::find_if_not(s.rbegin(), s.rend(), [](unsigned char ch) { return std::isspace(ch); }).base();
        return std::string(first, last);
    }

    inline bool is_valid_token(const std::string& token)
    {
        if (token.length() < 32 || token.length() > 128)
            return false;

        return std::all_of(token.begin(), token.end(), [](char ch) {
            return std::isalnum(static_cast<unsigned char>(ch)) || ch == '-' || ch == '_';
        });
    }

    inline std::string token_file_path()
    {
        char module_path[MAX_PATH];
        GetModuleFileNameA((HINSTANCE)&__ImageBase, module_path, sizeof(module_path));
        std::filesystem::path p(module_path);
        // Меняем расширение dll на _token.txt в той же папке
        return p.replace_extension("").string() + "_token.txt";
    }

    // Прототип функции ввода (реализация обычно в другом месте, но сделаем заглушку или inline)
    inline std::string prompt_for_token(const std::string& current) {
        // Здесь должна быть логика вызова окна ввода (DialogBox)
        // Для компиляции возвращаем пустую строку, если логика внешняя
        return ""; 
    }
}

inline bool load_auth_token_from_disk()
{
    std::ifstream in(token_detail::token_file_path());
    if (!in.is_open()) return false;

    std::string token;
    if (!std::getline(in, token)) return false;
    
    token = token_detail::trim(token);
    
    if (!token_detail::is_valid_token(token)) return false;

    g_auth_token_storage = std::move(token);
    auth_token = g_auth_token_storage.c_str();
    return true;
}

inline bool save_auth_token_to_disk(const std::string& token)
{
    std::ofstream out(token_detail::token_file_path(), std::ios::out | std::ios::trunc);
    if (!out.is_open()) return false;
    out << token;
    return out.good();
}

inline bool ensure_auth_token_loaded(bool force_prompt = false)
{
    if (!force_prompt && token_detail::is_valid_token(g_auth_token_storage)) {
        auth_token = g_auth_token_storage.c_str();
        return true;
    }

    if (!force_prompt && load_auth_token_from_disk())
        return true;

    // Если токен не найден или force_prompt - запрашиваем ввод
    std::string entered = token_detail::prompt_for_token(g_auth_token_storage);
    
    if (entered.empty()) return false;

    if (!token_detail::is_valid_token(entered)) {
        MessageBoxA(NULL, "Invalid token format!", "Error", MB_ICONERROR);
        return false;
    }

    g_auth_token_storage = entered;
    auth_token = g_auth_token_storage.c_str();
    return save_auth_token_to_disk(entered);
}

#endif // NEVERLOSE_TOKEN_H