#pragma once

#include <queue>
#include <array>
#include <vector>
#include <string>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <unordered_map>

#include "Node.hpp"
#include "Byte.hpp"
#include "Utils.hpp"
#include "Decoder.hpp"
#include "Encoder.hpp"
#include "Terminal.hpp"
#include "ByteWriter.hpp"
#include "ByteStream.hpp"
#include "FileReader.hpp"
#include "Config.hpp"

class Huffman
{
private:
    FileReader *reader;
    Encoder *enc;
    ByteWriter *writer;
    Decoder *dec;

public:
    Huffman(){
        reader = new FileReader();
        enc = new Encoder();
        dec = new Decoder();
        writer = new ByteWriter();
    }

    ~Huffman(){
        delete reader;
        delete enc;
        delete dec;
        delete writer;
    }

public:
    bool encode(std::string in_file_name, std::string out_file_name);

    bool decode(std::string in_file_name, std::string out_dir, std::string &out_file_name);
};
