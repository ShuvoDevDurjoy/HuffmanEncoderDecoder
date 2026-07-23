#pragma once

#include <array>

#include <iostream>
#include "FileReader.hpp"
#include "Node.hpp"
#include "../Utils/Type.hpp"
#include <cstdint>
#include <bit>
#include <algorithm>

struct Compare{
    bool operator()(Node* a, Node* b){
        return a->frequency > b->frequency;
    }
};

namespace Utils{

    int byte_count(int num){
        int count = 0;
        while(num > 0){
            num /= 256;
            count++;
        }
        return count;
    }
    
    bool read_uint8_t(FileReader* reader, uint8_t &byte){
        return reader->next_byte(byte);
    }

    bool read_uint16_t(FileReader* reader, uint16_t &bytes){
        uint8_t byte1, byte2;
        if (reader->next_byte(byte1) && reader->next_byte(byte2))
        {
            bytes = 0;
            bytes |= byte1;
            bytes = (bytes << 8) | byte2;
            return true;
        }
        return false;
    }

    bool read_uint64_t(FileReader *reader, uint64_t &f, size_t max_read = 8){
        size_t max_byte_read = 8;
        max_read = std::max(static_cast<size_t>(0), std::min(max_read, max_byte_read));
        uint8_t byte;
        f = 0;
        while (max_read && reader->next_byte(byte))
        {
            f <<= 8;
            f |= byte;
            max_read--;
        }

        return max_read == 0;
    }

    bool read_n_bytes_into_stream(FileReader* reader, ByteStream &bs, int n_bytes, bool sync = false){
        uint8_t byte;
        while (n_bytes && reader->next_byte(byte))
        {
            bs.push_byte(byte, sync);
            n_bytes--;
        }

        return n_bytes == 0;
    }

    uint8_t min_byte_count(uint64_t nc) {
        if (nc == 0) return 1;
        uint8_t bits = 64 - __builtin_clzll(nc);
        return (bits + 7) / 8;
    }

    bool uint16_t_to_bytestream(uint16_t nc, ByteStream &bs){
        uint16_t ext = 0 | 0xFF;
        ext <<= 8;
        for (int i = 0; i < 2; ++i)
        {
            uint8_t byte = (nc & ext) >> 8;
            nc <<= 8;
            bs.push_byte(byte);
        }
        return true;
    }

    bool frequency_to_stream(std::array<TYPE_FREQUENCY, BYTE_SIZE> &freq, ByteStream &bs, uint8_t min_bytes){
        uint64_t ext = 0 | 0xFF;
        ext <<= 56;
        for (size_t i = 0; i < freq.size(); ++i)
        {
            if(freq[i]){
                bs.push_byte(static_cast<uint8_t>(i));
                for (int x = min_bytes - 1; x >= 0; --x)
                {
                    uint8_t byte = (freq[i] >> (8 * x)) & 0xFF;
                    bs.push_byte(byte);
                }
            }
        }

        return bs.size()!=0;
    }

    void build_frequency_map(std::array<TYPE_FREQUENCY, BYTE_SIZE> &freq, uint16_t &num_unique_chars, uint8_t &min_num_bytes, FileReader* reader){
        // set initial frequency of all byte to 0
        freq.fill(0);
        num_unique_chars = 0;
        //variable to load next byte in
        TYPE_CHAR next_byte;
        //reader reads a file till end to build frequency map
        while(reader->next_byte(next_byte)){
            if(freq[next_byte]==0){
                num_unique_chars++;
            }
            freq[next_byte]++;
        }

        std::cout << (int)num_unique_chars << std::endl;

        auto max_freq_size = std::max_element(freq.begin(), freq.end());
        uint64_t max_frequency = *max_freq_size;

        min_num_bytes = min_byte_count(max_frequency);
    }

    Node* build_huffman_tree(std::array<TYPE_FREQUENCY, BYTE_SIZE> &freq)
    {
        std::priority_queue<Node*, std::vector<Node*>, Compare> min_heap;
        //initialize the min heap with leaf nodes
        for (size_t byte = 0; byte < freq.size(); ++byte){
            // check if the frequency of the byte is not 0
            if(freq[byte]){
                Node *node = new Node(byte, freq[byte]);
                min_heap.push(node);
            }
        }

        if(min_heap.size()==0)
            return nullptr;

        // untill the min heap is of size 1, means only the root node of the tree is left
        //retrieve two smaller frequency nodes and create a new node with frequency equal to their sum
        //and push that into the mean heap
        while(min_heap.size() > 1){
            //get the left node
            Node *left = min_heap.top();
            min_heap.pop();
            //get the right node
            Node *right = min_heap.top();
            min_heap.pop();
    
            //insert a new node of frequency equals sum of left and right node
            min_heap.push(new Node(left->frequency + right->frequency, left, right));
        }
    
        //At this point top value of min heap is the root node of the huffman tree
        Node* root = min_heap.top();
        min_heap.pop();
    
        //return the root node
        return root;
    }

    // performs dfs on the huffman tree until a leaf node is found
    //when a leaf node is found then set the code for the byte
    //it is guaranted that leaf nodes are byte node by initialization of priority queue
    void build_huffman_codes(Node *root, ByteStream &code, std::array<ByteStream, BYTE_SIZE> &huffman_codes)
    {
        //check if root is null or not
        //this is just a guard against malicious input
        if (root == nullptr)
            return;
        if (root->is_leaf())
        {
            huffman_codes[root->c] = code;
            return;
        }
        bool bit;

        //when we will go left then bit 0 will be inserted into code
        //and when we will go right then bit 1 will be inserted into code
        //after going left when we will go right then bit 0 will popped out and 
        //same for after doing right traversal(pop bit 1)

        //go left
        code.push_bit(false);
        build_huffman_codes(root->left, code, huffman_codes);
        //back from left
        code.pop_bit(bit);
        //go right
        code.push_bit(true);
        build_huffman_codes(root->right, code, huffman_codes);
        //back from right
        code.pop_bit(bit);
    }
}