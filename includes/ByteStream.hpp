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
    size_t byte_size = -1;

public:
    ByteStream() {}

    void push_bit(bool bit);

    bool pop_bit(bool &bit);

    void flush();

    void push_byte(Byte b, bool sync = false);

    void push_byte(uint8_t byte, bool sync = false);

    void push_stream(ByteStream bs, bool sync = false);
    bool pop_byte(Byte &byte);

    bool is_empty()
    {
        return data.size() == 0;
    }

    void padding_back(uint8_t padding)
    {
        if (!is_empty())
        {
            data.back().shift_right(padding);
        }
    }

    void test_state();

    size_t get_pad()
    {
        if (!is_empty())
        {
            return data.back().padding;
        }
        return 0;
    }

    size_t size()
    {
        return data.size();
    }

    void to_string()
    {
        size_t t = data.size();

        while (t)
        {
            std::cout << " " << std::bitset<8>(static_cast<int>(data.front().get_byte())) << " and pad: " << (int)data.front().padding << " ";
            Byte front = data.front();
            data.pop_front();
            push_byte(front);
            t--;
        }
    }
};