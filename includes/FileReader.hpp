#pragma once

#include <cstdint>
#include <fstream>
#include <iostream>
#include <array>
#include "../Utils/Type.hpp"

class FileReader{
private:
    std::ifstream file;
    std::array<TYPE_CHAR, BUFFER_SIZE> buffer;
    std::streamsize bytes_read = 0;
    std::size_t index = 0;

private:
    bool can_read();
    bool is_buffer_empty();

public:
    FileReader() {};

    bool is_open() const;
    void close();
    bool open(std::string file_name);
    bool reset(){
        if(!is_open()){
            return false;
        };
        file.clear();
        file.seekg(0, std::ios::beg);
        bytes_read = 0;
        index = 0;
        return !file.fail() && file.tellg() == std::streampos(0);
    }

public:
    bool read_chunk();
    bool next_byte(uint8_t &byte);
};