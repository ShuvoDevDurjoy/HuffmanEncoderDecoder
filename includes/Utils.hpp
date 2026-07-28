#pragma once

#include <bit>
#include <array>
#include <bitset>
#include <string>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <cstdint>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <system_error>

#include "Type.hpp"
#include "Node.hpp"
#include "Terminal.hpp"
#include "ByteStream.hpp"
#include "FileReader.hpp"

class FileReader;

struct Compare
{
    bool operator()(Node *a, Node *b)
    {
        return a->frequency > b->frequency;
    }
};

namespace Utils
{
    std::string trim(const std::string &value);
    std::string get_extension(const std::filesystem::path &p);
    std::string get_truncated_file_name(const std::filesystem::path &p);
    std::string get_truncated_file_name_from_path(const std::filesystem::path &p);
    std::string to_lower(std::string value);
    bool file_exists(const std::string &path);
    bool file_exists(const std::filesystem::path &path);
    bool dir_exists(const std::string &path);
    bool convert_string_to_stream(const std::string content, ByteStream &bs);
    bool convert_stream_to_string(ByteStream &bs, std::string &st);
    bool output_parent_exists(const std::string &path);
    bool parent_dir(const std::filesystem::path &path, std::filesystem::path &parent_path);
    uintmax_t file_size(const std::string &path);
    std::string format_file_size(uintmax_t bytes);
    bool encoded_output_path(const std::filesystem::path &input_file, std::string &output_file);
    std::string format_size_change(uintmax_t input_size, uintmax_t output_size);
    int byte_count(int num);
    bool read_uint8_t(FileReader *reader, std::uint8_t &byte);
    bool read_uint16_t(FileReader *reader, uint16_t &bytes);
    bool read_uint64_t(FileReader *reader, uint64_t &f, size_t max_read = 8);
    bool read_n_bytes_into_stream(FileReader *reader, ByteStream &bs, size_t n_bytes, bool sync = false);
    uint8_t min_byte_count(uint64_t nc);
    bool uint16_t_to_bytestream(uint16_t nc, ByteStream &bs);
    bool read_string(FileReader *reader, std::string &out_string);
    bool frequency_to_stream(std::array<TYPE_FREQUENCY, BYTE_SIZE> &freq, ByteStream &bs, uint8_t min_bytes);
    void build_frequency_map(std::array<TYPE_FREQUENCY, BYTE_SIZE> &freq, uint16_t &num_unique_chars, uint8_t &min_num_bytes, FileReader *reader, bool show_terminal = true);
    Node *build_huffman_tree(std::array<TYPE_FREQUENCY, BYTE_SIZE> &freq);
    void build_huffman_codes(Node *root, ByteStream &code, std::array<ByteStream, BYTE_SIZE> &huffman_codes);
};