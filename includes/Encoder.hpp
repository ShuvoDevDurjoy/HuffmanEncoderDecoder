#pragma once

#include <fstream>
#include <array>
#include <cstdlib>
#include <iostream>
#include "../Utils/Type.hpp"
#include "ByteStream.hpp"
#include "FileReader.hpp"
#include "ByteWriter.hpp"
#include "Byte.hpp"
#include <string>
#include "File.hpp"
#include "Utils.hpp"
#include <algorithm>

class Encoder
{
private:
    ByteWriter *out_file;
    FileReader *in_file;
    uint8_t version = 1;
    uint16_t num_unique_bits = 0;

    std::array<TYPE_CHAR, BYTE_SIZE> frequencies;
    std::array<ByteStream, BYTE_SIZE> codes;
    std::uint8_t padding = 0;

private:
    bool _encode(EncodedContent &content);
    bool _encode_content(std::array<ByteStream, BYTE_SIZE> &codes, ByteStream &bs);
    void _build_frequency_map(std::array<TYPE_FREQUENCY, BYTE_SIZE> &freq, uint16_t &num_unique_chars, uint8_t &min_num_bytes, FileReader *reader);

public:
    Encoder()
    {
        in_file = new FileReader();
        out_file = new ByteWriter();
    };

    bool encode(std::string in_file, std::string out_file);
};