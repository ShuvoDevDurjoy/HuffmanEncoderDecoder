#pragma once

#include <array>
#include <queue>
#include <cstdint>
#include <iostream>
#include "Byte.hpp"

class ByteStream
{
private:
    std::deque<Byte> data;

public:
    ByteStream() {}

    void push_bit(bool bit);

    bool pop_bit(bool &bit);

    void push_byte(Byte b, bool sync = false);

    void push_byte(uint8_t byte, bool sync = false);

    void push_stream(ByteStream bs, bool sync = false);
    bool pop_byte(Byte &byte);

    bool is_empty();

    void padding_back(uint8_t padding);

    size_t get_pad() const;

    bool get_bit(size_t index, bool &bit) const;

    size_t size() const;

    void to_string();
};