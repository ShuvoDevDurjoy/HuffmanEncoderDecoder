#include "../includes/ByteStream.hpp"



void ByteStream::push_bit(bool bit)
{
    if(!is_empty() && !data.back().is_full()){
        data.back().push(bit);
    }
    else{
        Byte b = Byte();
        b.push(bit);
        data.push_back(b);
    }
}

bool ByteStream::pop_bit(bool &bit)
{
    if(!is_empty()){
        while(!data.empty() && data.back().is_empty()){
            data.pop_back();
        }

        if(!data.empty()){
            data.back().pop(bit);
            if(data.back().is_empty()){
                data.pop_back();
            }
            return true;
        }
    }

    return false;
}

void ByteStream::push_byte(uint8_t byte, bool sync){
    push_byte(Byte(byte), sync);
}

void ByteStream::push_byte(Byte b, bool sync){
    if(sync){
        bool bit = false;
        while (!b.is_empty())
        {
            b.pop_bit_front(bit);
            push_bit(bit);
        }
    }
    else{
        data.push_back(b);
    }
}

bool ByteStream::pop_byte(Byte &byte){
    if(!is_empty()){
        byte = data.front();
        data.pop_front();
        return true;
    }
    return false;
}

void ByteStream::push_stream(ByteStream bs, bool sync){
    while(!bs.is_empty()){
        Byte b;
        bs.pop_byte(b);
        push_byte(b, sync);
    }
}


bool ByteStream::is_empty()
{
    return data.size() == 0;
}

void ByteStream::padding_back(uint8_t padding)
{
    if (!is_empty())
    {
        data.back().shift_right(padding);
    }
}

size_t ByteStream::get_pad() const
{
    if (!data.empty())
    {
        return data.back().padding;
    }
    return 0;
}

bool ByteStream::get_bit(size_t index, bool &bit) const
{
    const size_t bit_count = data.size() * 8 - (data.empty() ? 0 : data.back().padding);
    if (index >= bit_count)
    {
        return false;
    }

    const uint8_t byte = data[index / 8].get_byte();
    bit = ((byte >> (7 - (index % 8))) & 1) != 0;
    return true;
}

size_t ByteStream::size() const
{
    return data.size();
}

void ByteStream::to_string()
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