#pragma once

#include <queue>
#include <array>
#include <fstream>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "Node.hpp"
#include "Content.hpp"
#include "ByteStream.hpp"
#include "ByteWriter.hpp"
#include "FileReader.hpp"
#include "Config.hpp"
#include "Utils.hpp"

class Decoder
{
private:
    struct ArchiveEntry
    {
        bool is_directory;
        std::string output_path;
        uint64_t size;
    };

    struct ArchiveSymlink
    {
        std::string output_path;
        std::string target;
    };

private:
    ByteWriter *writer;
    FileReader *reader;
    uint8_t version_number = 1;
    uint8_t archive_version = 0;
    std::string decoder_magic_code = MAGIC_CODE;
    std::vector<ArchiveEntry> archive_entries;
    std::vector<ArchiveSymlink> archive_symlinks;

public:
    Decoder();
    ~Decoder();

private:
    bool pretest(std::string &in_file, std::string &out_dir);

    bool read_metadata(const std::string &out_dir, std::string &out_path);

    bool on_fail();

    bool _read_chars(std::array<TYPE_FREQUENCY, BYTE_SIZE> &freq, uint16_t uni_chars, uint8_t max_freq_byte_size);

    bool _decode_stream(Node *root, uint64_t expected_size, uint8_t padding,
                        const std::vector<ArchiveEntry> *entries = nullptr);

public:
    bool decode(std::string in_file, std::string &out_dir, std::string &out_path);
};
