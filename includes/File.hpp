#pragma once

#include <cstdint>
#include <vector>
#include "ByteStream.hpp"
#include <array>

struct CharAndFrequency{
    uint8_t c = 0;
    ByteStream bt;
};

struct EncodedContent{
    ByteStream magic_code;
    uint8_t version_number;
    uint16_t file_name_size;
    ByteStream file_name_bytes;
    ByteStream num_unique_chars;
    uint8_t max_freq_byte_size;
    ByteStream frequency_codes;
    uint8_t padding;
    ByteStream content_codes;
};


struct DecodedContent{
    ByteStream magic_code;
    uint8_t version_number;
    uint16_t file_name_size;
    ByteStream file_name_bytes;
    uint16_t num_unique_chars;
    uint8_t max_freq_byte_size;
    uint8_t padding;
    ByteStream content_codes;
};