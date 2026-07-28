#include "../includes/Encoder.hpp"

#include <algorithm> // For std::max

bool Encoder::_encode_content(std::array<ByteStream, BYTE_SIZE> &codes, ByteStream &bs)
{
    //Get total bytes from input file
    size_t total_bytes = in_file->size();
    // Handle empty file edge case
    if (total_bytes == 0)
    {
        Terminal::run_with_progress("Encoding Content", 100);
        return true;
    }
    //step_size is at least 1 byte to prevent division by zero
    size_t step_size = std::max<size_t>(1, total_bytes / 100);

    size_t read_bytes = 0;
    uint8_t last_progress = 0;

    Terminal::run_with_progress("Encoding Content", 0);

    uint8_t byte;
    while (in_file->next_byte(byte))
    {
        bs.push_stream(codes[byte], true);
        read_bytes++;

        // if a percent is read then trigger progress update
        if (read_bytes % step_size == 0)
        {
            uint8_t current_progress = static_cast<uint8_t>((read_bytes * 100) / total_bytes);

            // Only update terminal UI if percentage actually changed
            if (current_progress > last_progress)
            {
                Terminal::run_with_progress("Encoding Content", current_progress);
                last_progress = current_progress;
            }
        }
    }

    // Ensure terminal shows 100% when loop finishes
    Terminal::run_with_progress("Encoding Content", 100);
    std::cout << std::endl;

    return true;
}

bool Encoder::_encode(EncodedContent& content){
    out_file->write(content.magic_code_size);
    out_file->write(content.magic_code);
    out_file->write(content.version_number);
    out_file->write(content.file_name_size);
    out_file->write(content.file_name_bytes);
    out_file->write(content.num_unique_chars);
    out_file->write(content.max_freq_byte_size);
    out_file->write(content.frequency_codes);
    out_file->write(content.padding);
    out_file->write(content.content_codes, true);
    return true;
}

bool Encoder::encode(std::string in_file, std::string out_file)
{
    if(!Utils::file_exists(in_file))
        return false;
    this->in_file->open(in_file);
    this->out_file->open(out_file);

    if(!(this->in_file->is_open() && this->out_file->is_open())){
        this->in_file->close();
        this->out_file->close();
        return false;
    }

    EncodedContent content;
    content.version_number = 1;

    std::filesystem::path file_path;
    bool success;
    success = this->in_file->get_file_path(file_path);
    if(!success)
        return false;

    Utils::convert_string_to_stream(Utils::get_truncated_file_name_from_path(file_path), content.file_name_bytes);
    content.file_name_size = content.file_name_bytes.size();

    Utils::convert_string_to_stream(this->encoder_magic_code, content.magic_code);
    content.magic_code_size = static_cast<uint8_t>(content.magic_code.size());

    std::array<uint64_t, BYTE_SIZE> freq;
    uint16_t num_unique_chars;
    Utils::build_frequency_map(freq, num_unique_chars, content.max_freq_byte_size, this->in_file);
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
    success = _encode(content);

    this->in_file->close();
    this->out_file->close();
    return success;
}
