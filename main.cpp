#include <iostream>
#include "includes/ByteStream.hpp"
#include "includes/ByteWriter.hpp"
#include "src/ByteStream.cpp"
#include "src/ByteWriter.cpp"
#include "includes/Byte.hpp"
#include "includes/Huffman.hpp"
#include "includes/Encoder.hpp"
#include "src/Encoder.cpp"
#include <bitset>
using namespace std;

int main(){

    std::string input_file_path = "/home/shuvo/Downloads/cse332_13.04.25.mp4";
    std::string encoded_binary_path = "/home/shuvo/Downloads/cse332_13.04.25.bin";
    std::string decoded_binary_path = "/home/shuvo/Downloads/decoded.mp4";

    Huffman *huf = new Huffman();
    huf->encode(input_file_path, encoded_binary_path);
    // huf->decode("files/encoded/test.bin", "files/original/test_decoded.txt");

    // huf->show_frequency_map();

    return 0;
}