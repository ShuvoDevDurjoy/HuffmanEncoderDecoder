#include "../includes/Encoder.hpp"

void Encoder::_build_frequency_map(std::array<TYPE_FREQUENCY, BYTE_SIZE> &freq, uint16_t &num_unique_chars, uint8_t &min_num_bytes, FileReader* reader){
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
    auto max_freq_size = std::max_element(freq.begin(), freq.end());
    uint64_t max_frequency = *max_freq_size;
    min_num_bytes = Utils::min_byte_count(max_frequency);
}


bool Encoder::_encode_content(std::array<ByteStream, BYTE_SIZE> &codes, ByteStream &bs)
{
    uint8_t byte;
    // std::cout << "File is open of close: " << in_file->is_open() << std::endl;
    while (in_file->next_byte(byte))
    {
        bs.push_stream(codes[byte], true);
    }

    return true;
}

bool Encoder::_encode(EncodedContent& content){
    out_file->write(content.version_number);
    out_file->write(content.num_unique_chars);
    out_file->write(content.max_freq_byte_size);
    out_file->write(content.frequency_codes);
    out_file->write(content.padding);
    out_file->write(content.content_codes);
    return true;
}

bool Encoder::encode(std::string in_file, std::string out_file)
{
    this->in_file->open(in_file);
    this->out_file->open(out_file);

    if(!(this->in_file->is_open() && this->out_file->is_open())){
        this->in_file->close();
        this->out_file->close();
        return false;
    }

    EncodedContent content;
    content.version_number = 1;

    std::array<uint64_t, BYTE_SIZE> freq;
    uint16_t num_unique_chars;
    this->_build_frequency_map(freq, num_unique_chars, content.max_freq_byte_size, this->in_file);
    Utils::uint16_t_to_bytestream(num_unique_chars, content.num_unique_chars);
    Utils::frequency_to_stream(freq, content.frequency_codes, content.max_freq_byte_size);

    this->in_file->reset();
    Node* root = Utils::build_huffman_tree(freq);
    if(!root){
        this->in_file->close();
        this->out_file->close();
        return false;
    }

    std::array<ByteStream, BYTE_SIZE> huffman_codes;
    ByteStream code;
    Utils::build_huffman_codes(root, code, huffman_codes);
    this->_encode_content(huffman_codes, content.content_codes);
    content.padding = content.content_codes.get_pad();
    bool success = _encode(content);

    this->in_file->close();
    this->out_file->close();
    return success;
}
