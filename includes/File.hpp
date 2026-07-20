#pragma once

#include <cstdint>
#include <vector>
#include "ByteStream.cpp"

struct CharAndFrequency{
    uint8_t c = 0;
    ByteStream bt;
};

struct EncodedContent{
    ByteStream magic_code;
    uint8_t version_number;
    uint16_t file_name_size;
    ByteStream file_name_bytes;
    uint16_t num_unique_chars;
    uint16_t max_freq_byte_size;
    std::array<ByteStream, BYTE_SIZE> frequency_codes;
    uint8_t padding;
    ByteStream content_codes;
};
