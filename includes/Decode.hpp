#pragma once

#include <fstream>
#include <iostream>
#include <array>
#include <queue>
#include "ByteWriter.hpp"
#include "FileReader.hpp"
#include "../Utils/Type.hpp"
#include <cstdint>
#include "Node.hpp"
#include "ByteStream.hpp"

class Decode
{
private:
    ByteWriter *writer;
    FileReader *reader;
    uint8_t version_number;

public:
    Decode()
    {
        writer = new ByteWriter();
        reader = new FileReader();
    }

private:
    struct Compare
    {
        bool operator()(Node *a, Node *b)
        {
            return a->frequency > b->frequency;
        }
    };

private:
    bool _read_uint64(uint64_t &d)
    {
        uint8_t iter = 128;
        uint8_t byte;
        d = 0;
        while (iter && reader->next_byte(byte))
        {
            d <<= 8;
            d |= byte;
            iter >>= 1;
        }
        return iter == 0;
    }

private:
    bool _read_version_number()
    {
        // std::cout << "next byte read" << std::endl;
        uint8_t byte;
        if (reader->next_byte(byte))
        {
            version_number = static_cast<int>(byte);
            return true;
        }
        return false;
    }

    bool _read_number_of_unique_chars(uint16_t &uni_chars)
    {
        uint8_t byte1, byte2;
        if (reader->next_byte(byte1) && reader->next_byte(byte2))
        {
            uni_chars = 0;
            uni_chars |= byte1;
            uni_chars = (uni_chars << 8) | byte2;
            return true;
        }
        return false;
    }

    bool _read_chars(std::array<uint64_t, BYTE_SIZE> &freq, uint16_t uni_chars)
    {
        int num_iter = static_cast<int>(uni_chars);
        uint8_t byte = 0;
        uint64_t f = 0;
        for (int i = 0; i < num_iter; ++i)
        {
            if (reader->next_byte(byte) && _read_uint64(f))
            {
                freq[byte] = f;
            }
            else
            {
                return false;
            }
        }
        return true;
    }

    void traverse(Node *root, ByteStream &code)
    {
        // code.to_string();
        if (root == nullptr)
            return;
        if (root->is_leaf())
        {
            // huffman_codes[root->c] = code;
            std::cout << "root value: " << (int)root->c << std::endl;
            code.to_string();
            std::cout << std::endl;
            return;
        }
        bool bit;
        code.push_bit(false);
        traverse(root->left, code);
        code.pop_bit(bit);
        code.push_bit(true);
        traverse(root->right, code);
        code.pop_bit(bit);
    }

    Node *_build_huffman_code_tree(std::array<uint64_t, BYTE_SIZE> &freq)
    {
        std::priority_queue<Node *, std::vector<Node *>, Compare> min_heap;
        for (size_t i = 0; i < freq.size(); ++i)
        {
            if (freq[i])
            {
                min_heap.push(new Node(i, freq[i]));
            }
        }

        while (min_heap.size() > 1)
        {
            Node *left = min_heap.top();
            min_heap.pop();
            Node *right = min_heap.top();
            min_heap.pop();

            min_heap.push(new Node(left->frequency + right->frequency, left, right));
        }

        Node *root = min_heap.top();
        min_heap.pop();

        return root;
    }

    void _read_code_stream(ByteStream &bs)
    {
        uint8_t byte;
        while (reader->next_byte(byte))
        {
            bs.push_byte(byte);
        }
    }

    void _decode(ByteStream &bs, Node *root, ByteStream &content)
    {
        Node *current = root;

        Byte byte;
        bool bit;
        // bs.to_string();

        while (!bs.is_empty())
        {
            bs.pop_byte(byte);

            while (!byte.is_empty())
            {
                byte.pop_bit_front(bit);

                if (bit)
                    current = current->right;
                else
                    current = current->left;

                if (current->is_leaf())
                {
                    content.push_byte(current->c);
                    current = root;
                }
            }
        }
    }

    void show_freq(std::array<uint64_t, BYTE_SIZE> &freq){
        for (size_t i = 0; i < freq.size(); ++i){
            if(freq[i]){
                std::cout << "Byte is: " << (int)i << " and frequency: "<< (int)freq[i] << std::endl;
            }
        }
    }

public:
    bool decode(std::string in_file, std::string out_file)
    {
        std::cout << "******* Starting Decoding ************" << std::endl;
        writer->open(out_file);
        reader->open(in_file);


        if (writer->is_open() && reader->is_open())
        {

            std::array<uint64_t, BYTE_SIZE> freq = {};
            uint16_t uni_char = 0;
            uint8_t padding = 0;
            bool success = false;
            success = _read_version_number();
            // std::cout << "Version number: " << (int)version_number << " in byte: " << std::bitset<8>(version_number) << std::endl;
            if (!success)
            return false;
            success = _read_number_of_unique_chars(uni_char);
            // std::cout << "Unique Chars: " << (int)uni_char << " in byte: " << std::bitset<16>(uni_char) << std::endl;
            if (!success)
            return false;
            success = _read_chars(freq, uni_char);
            // show_freq(freq);
            if (!success)
            return false;
            success = reader->next_byte(padding);
            // std::cout << "padding is: " << (int)padding << std::endl;
            // std::cout << "Done so far" << std::endl;
            if (!success)
                return false;
            Node *root = _build_huffman_code_tree(freq);
            ByteStream code;
            // traverse(root, code);
            ByteStream bs;
            _read_code_stream(bs);
            bs.padding_back(padding);
            ByteStream content;
            _decode(bs, root, content);
            writer->write(content);
        }

        writer->close();
        reader->close();

        return true;
    }
};