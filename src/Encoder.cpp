#include "../includes/HuffmanCodes.hpp"

bool Encoder::_write_version_number()
{
    Byte v = Byte(version);
    out_file->write(v);
    return true;
}

bool Encoder::_write_num_unique_chars(uint16_t nc)
{
    uint16_t n = nc;
    ByteStream bn = ByteStream();
    uint16_t ext = 0 | 0xFF;
    ext <<= 8;
    for (int i = 0; i < 2; ++i)
    {
        uint8_t byte = (nc & ext) >> 8;
        nc <<= 8;
        bn.push_byte(byte);
    }
    // std::cout << std::endl;
    out_file->write(bn);
    return true;
}

bool Encoder::_write_unique_codes(std::array<uint64_t, BYTE_SIZE> &freq){
    uint64_t ext = 0 | 0xFF;
    ext <<= 56;
    for (size_t i = 0; i < freq.size(); ++i)
    {
        if(freq[i]){
            Byte b = Byte(static_cast<uint8_t>(i));
            out_file->write(b);
            ByteStream bs;
            for (int x = 0; x < 8; ++x)
            {
                uint64_t t = freq[i] << (8 * x);
                uint8_t byte = ((t & ext) >> 56) & 0xFF;
                bs.push_byte(byte);
            }
            // std::cout << "Writing frequency: " << freq[i] << std::endl;
            // bs.to_string();
            // std::cout << "Byte is: " << (int)i << std::endl;
            // std::cout << "Frequency is: " << freq[i] << std::endl;
            // std::cout<<"In byte: ";
            // bs.to_string();
            // std::cout << std::endl;
            out_file->write(bs);
        }
    }
    return true;
}

bool Encoder::_write_content_of_file(std::array<ByteStream, BYTE_SIZE> &codes)
{
    ByteStream bs;
    uint8_t byte;
    // std::cout << "File is open of close: " << in_file->is_open() << std::endl;
    while (in_file->next_byte(byte))
    {
        bs.push_stream(codes[byte], true);
    }
    // bs.to_string();
    // std::cout << std::endl;
    uint8_t padding = bs.get_pad();
    // std::cout << "Padding is: " << (int)padding << std::endl;
    Byte b = Byte(padding);
    out_file->write(b);
    out_file->write(bs);
    return true;
}

bool Encoder::encode(std::string in_file, std::string out_file, std::array<uint64_t, BYTE_SIZE> &freq, std::array<ByteStream, BYTE_SIZE> &codes, uint16_t num_uni_chars)
{
    // std::cout << "\n******Starting encoding******\n" << std::endl;
    this->in_file->open(in_file);
    this->out_file->open(out_file);
    if(this->in_file->is_open() && this->out_file->is_open()){
        this->_write_version_number();
        // std::cout << "Done with writing version number" << std::endl;
        this->_write_num_unique_chars(num_uni_chars);
        // std::cout << "Done with writing version number" << std::endl;
        this->_write_unique_codes(freq);
        this->_write_content_of_file(codes);
    }
    this->in_file->close();
    this->out_file->close();
    return true;
}