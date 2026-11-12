#pragma once

#include <array>
#include <string>
#include <fstream>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <vector>

#include "Content.hpp"
#include "Byte.hpp"
#include "Terminal.hpp"
#include "ByteStream.hpp"
#include "FileReader.hpp"
#include "ByteWriter.hpp"
#include "Config.hpp"
#include "Utils.hpp"

class Encoder
{
private:
    std::string encoder_magic_code = MAGIC_CODE;
    ByteWriter *out_file;
    FileReader *in_file;
    uint8_t version = 1;
    uint16_t num_unique_bits = 0;

    std::array<TYPE_FREQUENCY, BYTE_SIZE> frequencies;
    std::array<ByteStream, BYTE_SIZE> codes;
    std::uint8_t padding = 0;
private:
    uint8_t progress;
    uint8_t read_so_far = 0;

private:
    bool _encode(EncodedContent &content);
    bool _encode_content(std::array<ByteStream, BYTE_SIZE> &codes, size_t total_bytes);
    bool _encode_directory(const std::filesystem::path &directory, const std::string &output_path);
    uint8_t _get_padding(const std::array<TYPE_FREQUENCY, BYTE_SIZE> &freq,
                         const std::array<ByteStream, BYTE_SIZE> &codes);

public:
    Encoder()
    {
        in_file = new FileReader();
        out_file = new ByteWriter();
    };

    ~Encoder()
    {
        delete in_file;
        delete out_file;
    }

    bool encode(std::string in_file, std::string out_file);

    void onEnd(){

    }
};