#pragma once

#include <chrono>
#include <future>
#include <string>
#include <iostream>
#include <iomanip>
#include <thread>

namespace Color
{
    const std::string RESET = "\033[0m";
    const std::string BOLD = "\033[1m";
    const std::string DIM = "\033[2m";

    // Foreground Colors
    const std::string RED = "\033[31m";
    const std::string GREEN = "\033[32m";
    const std::string YELLOW = "\033[33m";
    const std::string BLUE = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN = "\033[36m";
    const std::string WHITE = "\033[37m";
}

namespace Terminal{

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

    inline void stat(const std::string &label, const std::string &value)
    {
        std::cout << "  " << Color::CYAN << std::left << std::setw(18)
                  << label << Color::RESET << value << std::right << "\n";
    }

    inline std::string prompt(const std::string &label)
    {
        std::string value;
        std::cout << Color::BOLD << label << Color::RESET << " ";
        std::getline(std::cin, value);
        return value;
    }

    inline void draw_progress_bar(size_t current, size_t total, const std::string &label = "", int bar_width = 34)
    {
        if (total == 0)
            return;

        float percentage = static_cast<float>(current) / total;
        if(percentage > 1.0f)
            percentage = 1.0f;
        int pos = static_cast<int>(bar_width * percentage);

        std::cout << "\r" << Color::DIM << std::setw(12) << std::left << label
                  << Color::RESET << Color::CYAN << "[" << Color::GREEN;

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

    template <typename Action>
    bool run_with_progress(const std::string &label, Action action)
    {
        auto task = std::async(std::launch::async, action);
        size_t progress = 5;

        while(task.wait_for(std::chrono::milliseconds(90)) != std::future_status::ready)
        {
            draw_progress_bar(progress, 100, label);
            if(progress < 95)
                progress++;
        }

        bool success = task.get();
        draw_progress_bar(100, 100, label);
        std::cout << "\n";
        return success;
    }
}
