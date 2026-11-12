#pragma once

#include <fstream>

#include "Byte.hpp"
#include "Terminal.hpp"
#include "ByteStream.hpp"

class ByteWriter{

private:
    std::ofstream file;

    
private: 
    bool _write(Byte &byte);
    bool _write(uint8_t byte);
    
public: 
    bool open(std::string file_name);
    bool is_open();
    void close();

    bool write(Byte &b);

    bool write(uint8_t &c);

    bool write_buffer(const uint8_t *buffer, size_t size);

    bool write(ByteStream &bs, bool show_terminal = false);
};
