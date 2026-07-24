#pragma once

#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <string>
#include <fstream>
#include <array>
#include "Utils.hpp"
#include "../Utils/Type.hpp"
#include "FileReader.hpp"
#include "../src/FileReader.cpp"
#include <cstdint>
#include "Byte.hpp"
#include "ByteWriter.hpp"
#include "ByteStream.hpp"
#include "Encoder.hpp"
#include "Node.hpp"
#include "Decode.hpp"

class Huffman
{
private:
    FileReader *reader;
    Encoder *enc;
    ByteWriter *writer;
    Decode *dec;

public:
    Huffman(){
        reader = new FileReader();
        enc = new Encoder();
        dec = new Decode();
        writer = new ByteWriter();
    }

public:
    bool encode(std::string in_file_name, std::string out_file_name){
        return this->enc->encode(in_file_name, out_file_name);
    }

    bool decode(std::string in_file_name, std::string out_file_name){
        return dec->decode(in_file_name, out_file_name);
    }

};
