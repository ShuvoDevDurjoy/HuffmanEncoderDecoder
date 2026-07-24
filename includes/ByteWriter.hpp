#pragma once

#include "Byte.hpp"
#include "ByteStream.hpp"
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
        _write(byte.get_byte());
        return true;
    }

    bool _write(uint8_t byte){
        if(is_open()){
            // std::cout<<" "<<std::bitset<8>(byte);
            file.write(reinterpret_cast<const char *>(&byte), sizeof(byte));
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

    bool write(uint8_t &c){
        return _write(c);
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
