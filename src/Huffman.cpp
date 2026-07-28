#include "../includes/Huffman.hpp"

bool Huffman::encode(std::string in_file_name, std::string out_file_name)
{
    return this->enc->encode(in_file_name, out_file_name);
}

bool Huffman::decode(std::string in_file_name, std::string out_dir, std::string &out_file_name)
{
    return dec->decode(in_file_name, out_dir, out_file_name);
}