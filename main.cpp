#include <iostream>
#include "includes/ByteStream.hpp"
#include "includes/ByteWriter.hpp"
#include "src/ByteStream.cpp"
#include "src/ByteWriter.cpp"
#include "includes/Byte.hpp"
#include "includes/Huffman.hpp"
#include "includes/HuffmanCodes.hpp"
#include "src/Encoder.cpp"
#include "src/Huffman.cpp"
#include <bitset>
using namespace std;

int main(){

    Huffman *huf = new Huffman();
    huf->encode("files/original/nasa (1).jpg", "files/encoded/test.bin");
    huf->decode("files/encoded/test.bin", "files/original/image.jpg");

    // huf->show_frequency_map();

    return 0;
}