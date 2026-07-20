#pragma once

#include "Byte.hpp"
#include <fstream>

class ByteWriter{

private:
    std::ofstream file;

public: 
    bool open(std::string file_name){
        file.open(file_name, std::ios::out | std::ios::binary);
        return is_open();
    }

    bool is_open(){
        return file.is_open();
    }

private: 
    bool _write(Byte &byte){
        if(is_open()){
            uint8_t b = byte.get_byte();
            file.write(reinterpret_cast<const char *>(&b), sizeof(b));
            return file.good();
        }
        return false;
    }

public:

    void close(){
        if(is_open()){
            file.close();
        }
    }

public: 
    bool write(Byte &b){
        return _write(b);
    }

    bool write(ByteStream &bs)
    {
        while (!bs.is_empty())
        {
            Byte b;
            bs.pop_byte(b);
            this->_write(b);
        }
        return true;
    }
};