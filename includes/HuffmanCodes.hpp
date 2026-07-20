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
    bool _write_version_number();
    bool _write_num_unique_chars(uint16_t nc);
    bool _write_unique_codes(std::array<uint64_t, BYTE_SIZE> &freq);

    bool _write_content_of_file(std::array<ByteStream, BYTE_SIZE> &codes);

public:
    Encoder()
    {
        in_file = new FileReader();
        out_file = new ByteWriter();
    };

    bool encode(std::string in_file, std::string out_file, std::array<uint64_t, BYTE_SIZE> &freq, std::array<ByteStream, BYTE_SIZE> &codes, uint16_t num_uni_chars = 0);
};