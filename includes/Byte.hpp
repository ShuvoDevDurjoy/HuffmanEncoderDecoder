#pragma once

#include <cstdint>
#include <iostream>
#include <bitset>

class Byte
{
public:
    std::uint8_t byte = 0;
    std::size_t padding = 8;

    Byte(){

    }

    Byte(uint8_t b){
        byte = b;
        padding = 0;
    }

    Byte(uint8_t b, size_t p){
        byte = b;
        padding = p;
    }

public:
    bool push(bool bit)
    {
        if (padding > 0)
        {
            byte = (byte << 1) | bit;
            padding--;
            return true;
        }
        return false;
    }

    void push(uint8_t p_byte, size_t p_padding = 0){
        byte = p_byte;
        padding = p_padding;
    }

    bool pop(bool &bit){
        if(is_empty()){
            return false;
        }
        bit = byte & 1;
        byte = byte >> 1;
        padding++;
        padding = (padding == 8 ? 8 : padding);
        return true;
    }

    void flush(){
        byte = 0;
        padding = 8;
    }

    bool is_empty(){
        return padding == 8;
    }

    bool is_full(){
        return padding == 0;
    }

    uint8_t get_byte(){
        return byte << padding;
    }

    bool pop_bit_front(bool &bit){
        if(padding < 8){
            uint8_t n_bit = 256 >> (padding + 1);
            bit = byte & n_bit;
            byte = byte ^ (byte & n_bit);
            padding++;
        }
        return false;
    }

    bool shift_right(uint8_t shift){
        byte >>= shift;
        padding = shift;
        padding = padding > 8 ? 8 : shift;
        return true;
    }

    void to_string(){
        std::cout << "Current Byte: " << std::bitset<8>(static_cast<int>(get_byte())) << " and padding: " << padding << std::endl;
    }
};
