#pragma once

#include <bitset>
#include <cstdint>
#include <iostream>

class Byte
{
public:
    std::uint8_t byte = 0;
    std::size_t padding = 8;

    Byte() {}

    Byte(uint8_t b);

    Byte(uint8_t b, size_t p);

public:
    bool push(bool bit);

    void push(uint8_t p_byte, size_t p_padding = 0);

    bool pop(bool &bit);

    void flush();

    bool is_empty();

    bool is_full();

    uint8_t get_byte();

    bool pop_bit_front(bool &bit);

    bool shift_right(uint8_t shift);

    void to_string();
};
