#pragma once

#include <queue>
#include <array>
#include <fstream>
#include <cstdint>
#include <iostream>

#include "Node.hpp"
#include "Content.hpp"
#include "ByteStream.hpp"
#include "ByteWriter.hpp"
#include "FileReader.hpp"
#include "Type.hpp"
#include "Utils.hpp"

class Decoder
{
private:
    ByteWriter *writer;
    FileReader *reader;
    uint8_t version_number = 1;
    std::string decoder_magic_code = "HUFF";

public:
    Decoder();

private:
    bool pretest(std::string &in_file, std::string &out_dir);

    bool read_metadata(const std::string &out_dir, std::string &out_path);

    bool on_fail();

    bool _read_chars(std::array<TYPE_FREQUENCY, BYTE_SIZE> &freq, uint16_t uni_chars, uint8_t max_freq_byte_size);

    void _read_code_stream(ByteStream &bs);

    bool _decode(ByteStream &bs, Node *root, ByteStream &content);

public:
    bool decode(std::string in_file, std::string &out_dir, std::string &out_path);
};
