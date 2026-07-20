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

void ByteStream::test_state()
{
    // std::cout << std::endl;
    int s = data.size();
    // std::cout << "Current States: "<<s << std::endl;
    while (s)
    {
        data.front().to_string();
        data.push_back(data.front());
        data.pop_front();
        s--;
    }
    // std::cout << std::endl;
}