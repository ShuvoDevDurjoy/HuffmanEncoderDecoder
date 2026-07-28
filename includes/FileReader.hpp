#pragma once

#include <array>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <filesystem>

#include "Config.hpp"
#include "Utils.hpp"

class FileReader
{
private:
    std::ifstream file;
    std::array<TYPE_CHAR, BUFFER_SIZE> buffer;
    std::streamsize bytes_read = 0;
    std::size_t index = 0;
    std::filesystem::path file_path;

private:
    bool can_read();
    bool is_buffer_empty();

public:
    FileReader() {};

    bool get_file_path(std::filesystem::path &path);
    bool is_open() const;
    void close();
    bool open(std::string file_name);
    bool reset();
    bool read_chunk();
    bool next_byte(uint8_t &byte);
    size_t size();
};