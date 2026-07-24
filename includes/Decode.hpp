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
#include "File.hpp"
#include "Utils.hpp"

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

    bool _read_chars(std::array<TYPE_FREQUENCY,BYTE_SIZE> &freq, uint16_t uni_chars, uint8_t max_freq_byte_size)
    {
        size_t num_iter = static_cast<size_t>(uni_chars);
        uint8_t byte = 0;
        uint64_t f = 0;
        for (size_t i = 0; i < num_iter; ++i)
        {
            f = 0;
            if (reader->next_byte(byte) && Utils::read_uint64_t(reader, f, max_freq_byte_size))
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
        if(!root)
            return;

        if(root->is_leaf())
        {
            Byte byte;
            bool bit = false;

            while (!bs.is_empty())
            {
                bs.pop_byte(byte);

                while (!byte.is_empty())
                {
                    byte.pop_bit_front(bit);
                    content.push_byte(root->c);
                }
            }
            return;
        }

        Node *current = root;

        Byte byte;
        bool bit = false;

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

public:
    bool decode(std::string in_file, std::string out_file)
    {
        writer->open(out_file);
        reader->open(in_file);

        auto fail = [&](){
            writer->close();
            reader->close();
            return false;
        };

        if (!(writer->is_open() && reader->is_open()))
        {
            return fail();
        }

        DecodedContent content = DecodedContent();
        std::array<TYPE_FREQUENCY, BYTE_SIZE> freq;
        freq.fill(0);

        bool success;
        success = Utils::read_uint8_t(reader, content.version_number);
        if (!success)
            return fail();
        success = Utils::read_uint16_t(reader, content.num_unique_chars);
        if (!success)
            return fail();
        success = Utils::read_uint8_t(reader, content.max_freq_byte_size);
        if (!success)
            return fail();
        success = _read_chars(freq, content.num_unique_chars, content.max_freq_byte_size);
        if (!success)
            return fail();
        success = reader->next_byte(content.padding);
        if (!success)
            return fail();

        Node *root = Utils::build_huffman_tree(freq);
        if(!root)
            return fail();

        ByteStream bs;
        _read_code_stream(bs);
        if(bs.size()==0)
            return fail();

        bs.padding_back(content.padding);
        _decode(bs, root, content.content_codes);
        success = writer->write(content.content_codes);

        writer->close();
        reader->close();

        return success;
    }
};
