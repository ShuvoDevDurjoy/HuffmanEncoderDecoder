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
#include "HuffmanCodes.hpp"
#include "Node.hpp"
#include "Decode.hpp"

class Huffman
{
private:
    FileReader *reader;
    Encoder *enc;
    ByteWriter *writer;
    Decode *dec;

private: 
    struct Compare{
        bool operator()(Node* a, Node* b){
            return a->frequency > b->frequency;
        }
    };

private:
    uint16_t num_unique_chars = 0;
    uint32_t max_frequency = 0;

private:
    std::string input_file_name;
    std::array<uint64_t, BYTE_SIZE> frequency = {};
    std::array<ByteStream, BYTE_SIZE> huffman_codes;
    std::priority_queue<Node *, std::vector<Node *>, Compare> min_heap;

public:
    Huffman(){
        reader = new FileReader();
        enc = new Encoder();
        dec = new Decode();
        writer = new ByteWriter();
    }

private:
    bool build_frequency_map();

    Node *build_tree();

    void build_huffman_codes(Node *root, ByteStream &code);

    void make_file();

    void _encode(std::string in_file_name, std::string out_file_name);

public:
    void encode(std::string in_file_name, std::string out_file_name);

    void decode(std::string in_file_name, std::string out_file_name){
        // std::cout << "Decoding in huffman" << std::endl;
        dec->decode(in_file_name, out_file_name);
    }

    void show_frequency_map(){
        for (size_t i = 0; i < frequency.size(); ++i){
            if(frequency[i]){
                std::cout << "Byte: " << i <<" and "<< (int)frequency[i] << std::endl;
            }
        }

        int index = 0;

        for(auto &it: huffman_codes){
            if(!it.is_empty()){
                std::cout << "index: " << index << std::endl;
                it.to_string();
            }
            index++;
        }
    }
};