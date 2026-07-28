#pragma once

#include <string>
#include <iomanip>
#include <iostream>
#include <string_view>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace Status
{

    namespace Modern
    {
        constexpr std::string_view SUCCESS = " \033[1;32m✔ SUCCESS\033[0m";
        constexpr std::string_view FAILED = " \033[1;31m✖ FAILED\033[0m";
        constexpr std::string_view CANCELLED = " \033[1;33m🚫 CANCELLED\033[0m";
        constexpr std::string_view PROGRESS = " \033[1;36m⏳ IN PROGRESS\033[0m";
        constexpr std::string_view CONTINUING = " \033[1;35m🔄 CONTINUING\033[0m";
        constexpr std::string_view WARNING = " \033[1;33m⚠️  WARNING\033[0m";
        constexpr std::string_view INFO = " \033[1;34mℹ INFO\033[0m";
        constexpr std::string_view PAUSED = " \033[1;33m⏸ PAUSED\033[0m";
        constexpr std::string_view RETRYING = " \033[1;35m⟳ RETRYING\033[0m";
    }

    namespace Badge
    {
        constexpr std::string_view SUCCESS = " \033[1;32m[  OK  ]\033[0m";
        constexpr std::string_view FAILED = " \033[1;31m[ FAIL ]\033[0m";
        constexpr std::string_view CANCELLED = " \033[1;33m[ CANC ]\033[0m";
        constexpr std::string_view PROGRESS = " \033[1;36m[ RUN  ]\033[0m";
        constexpr std::string_view CONTINUING = " \033[1;35m[ CONT ]\033[0m";
        constexpr std::string_view WARNING = " \033[1;33m[ WARN ]\033[0m";
        constexpr std::string_view INFO = " \033[1;34m[ INFO ]\033[0m";
        constexpr std::string_view PAUSED = " \033[1;33m[ WAIT ]\033[0m";
        constexpr std::string_view RETRYING = " \033[1;35m[ RETR ]\033[0m";
    }

    namespace Symbol
    {
        constexpr std::string_view SUCCESS = " \033[1;32m[✔]\033[0m";
        constexpr std::string_view FAILED = " \033[1;31m[✖]\033[0m";
        constexpr std::string_view CANCELLED = " \033[1;33m[⊘]\033[0m";
        constexpr std::string_view PROGRESS = " \033[1;36m[➜]\033[0m";
        constexpr std::string_view CONTINUING = " \033[1;35m[⟳]\033[0m";
    }
}

namespace Color
{
    const std::string_view RESET = "\033[0m";
    const std::string_view BOLD = "\033[1m";
    const std::string_view DIM = "\033[2m";

    // Foreground Colors
    const std::string_view RED = "\033[31m";
    const std::string_view GREEN = "\033[32m";
    const std::string_view YELLOW = "\033[33m";
    const std::string_view BLUE = "\033[34m";
    const std::string_view MAGENTA = "\033[35m";
    const std::string_view CYAN = "\033[36m";
    const std::string_view WHITE = "\033[37m";
}

namespace Terminal
{
    inline void init()
    {
#ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hOut != INVALID_HANDLE_VALUE)
        {
            DWORD dwMode = 0;
            if (GetConsoleMode(hOut, &dwMode))
            {
                dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(hOut, dwMode);
            }
        }
        HANDLE hErr = GetStdHandle(STD_ERROR_HANDLE);
        if (hErr != INVALID_HANDLE_VALUE)
        {
            DWORD dwMode = 0;
            if (GetConsoleMode(hErr, &dwMode))
            {
                dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                SetConsoleMode(hErr, dwMode);
            }
        }
#endif
    }

    inline void banner()
    {
        std::cout << Color::BOLD << Color::CYAN
                  << "\n"
                  << "  _   _        __  __ \n"
                  << " | | | |_   _ / _|/ _|\n"
                  << " | |_| | | | | |_| |_ \n"
                  << " |  _  | |_| |  _|  _|\n"
                  << " |_| |_|\\__,_|_| |_|  \n"
                  << Color::RESET << Color::DIM
                  << "  Huffman encoder and decoder\n\n"
                  << Color::RESET;
    }

    inline void section(const std::string &title)
    {
        std::cout << Color::BOLD << Color::BLUE << title << Color::RESET << "\n";
        std::cout << Color::DIM << std::string(52, '-') << Color::RESET << "\n";
    }

    inline void info(const std::string &message)
    {
        std::cout << Color::CYAN << "info: " << Color::RESET << message << "\n";
    }

    inline void success(const std::string &message)
    {
        std::cout << Color::GREEN << "success: " << Color::RESET << message << "\n";
    }

    inline void error(const std::string &message)
    {
        std::cerr << Color::RED << "error: " << Color::RESET << message << "\n";
    }

    inline void warning(const std::string &message)
    {
        std::cout << Color::YELLOW << "warning: " << Color::RESET << message << "\n";
    }

    // inline void stat(const std::string &label, const std::string &value)
    // {
    //     std::cout << "  " << Color::CYAN << std::left << std::setw(18)
    //               << label << Color::RESET << value << std::right << "\n";
    // }
    inline void stat(const std::string label, const std::string_view value)
    {
        std::cout << "   " << Color::CYAN << std::left << std::setw(18)
                  << label << Color::RESET << value << std::right << std::endl;
    }
    inline void stat_carriage(const std::string label, const std::string_view value)
    {
        std::cout << "\033[2K\r" << Color::CYAN << std::left << std::setw(50)
                  << label << Color::RESET << std::left << std::setw(30) << value << std::right;
    }

    inline std::string prompt(const std::string &label)
    {
        std::string value;
        std::cout << Color::BOLD << label << Color::RESET << " ";
        std::getline(std::cin, value);
        return value;
    }

    inline void draw_progress_bar(size_t current, size_t total, const std::string &label = "", std::string_view label_color = Color::DIM, std::string_view bar_color = Color::GREEN, int bar_width = 25)
    {
        if (total == 0)
            return;

        float percentage = static_cast<float>(current) / total;
        if (percentage > 1.0f)
            percentage = 1.0f;
        int pos = static_cast<int>(bar_width * percentage);

        std::cout << "\033[2K\r" << label_color << std::setw(30) << std::left << label
                  << Color::RESET << Color::CYAN << "[" << bar_color;

        for (int i = 0; i < bar_width; ++i)
        {
            if (i < pos)
                std::cout << "█";
            else if (i == pos)
                std::cout << ">";
            else
                std::cout << "░";
        }

        std::cout << Color::CYAN << "] "
                  << Color::BOLD << Color::YELLOW
                  << std::setw(3) << int(percentage * 100.0) << "% "
                  << Color::RESET << std::right << std::flush;
    }


    inline void run_with_progress(std::string label, uint8_t progress)
    {
        draw_progress_bar(progress, 100, label);
    }
}
