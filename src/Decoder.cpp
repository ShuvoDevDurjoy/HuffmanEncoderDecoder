#include "../includes/Decoder.hpp"

Decoder::Decoder()
{
    writer = new ByteWriter();
    reader = new FileReader();
}

Decoder::~Decoder()
{
    delete writer;
    delete reader;
}

bool Decoder::pretest(std::string &in_file, std::string &out_dir)
{
    if (!Utils::file_exists(in_file) || !Utils::dir_exists(out_dir))
    {
        return false;
    }

    reader->open(in_file);

    if (!(reader->is_open()))
        return on_fail();
    return true;
}

bool Decoder::read_metadata(const std::string &out_dir, std::string &out_path)
{
    std::string magic_code;
    uint8_t v_n;
    if (!Utils::read_string(reader, magic_code))
        return on_fail();
    if (magic_code != this->decoder_magic_code)
        return on_fail();
    if (!Utils::read_uint8_t(reader, v_n))
        return on_fail();
    if (v_n != this->version_number)
        return on_fail();

    std::string out_file_name;
    if (!Utils::read_string2(reader, out_file_name))
        return on_fail();

    out_path = (std::filesystem::path(out_dir) / out_file_name).string();
    writer->open(out_path);

    return true;
}

bool Decoder::on_fail()
{
    writer->close();
    reader->close();
    return false;
}

bool Decoder::_read_chars(std::array<TYPE_FREQUENCY, BYTE_SIZE> &freq, uint16_t uni_chars, uint8_t max_freq_byte_size)
{
    Terminal::stat_carriage("Building Frequency Table: ", Status::Modern::PROGRESS);
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
    Terminal::stat_carriage("Building Frequency Table: ", Status::Modern::SUCCESS);
    return true;
}

void Decoder::_read_code_stream(ByteStream &bs)
{
    Terminal::stat_carriage("Reading Encoded Content: ", Status::Modern::PROGRESS);
    uint8_t byte;
    while (reader->next_byte(byte))
    {
        bs.push_byte(byte);
    }
    Terminal::stat_carriage("Reading Encoded Content: ", Status::Modern::SUCCESS);
}

bool Decoder::_decode(ByteStream &bs, Node *root, ByteStream &content)
{
    if (!root)
        return false;

    if (root->is_leaf())
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
        return true;
    }

    Node *current = root;

    Byte byte;
    bool bit = false;

    size_t total_bytes = bs.size();
    if (total_bytes == 0)
    {
        Terminal::run_with_progress("Decoding Content", 100);
        return true;
    }

    // step_size is at least 1 byte to prevent division by zero
    size_t step_size = std::max<size_t>(1, total_bytes / 100);

    size_t read_bytes = 0;
    uint8_t last_progress = 0;

    Terminal::run_with_progress("Decoding Content", 0);

    while (!bs.is_empty())
    {
        bool pop_byte_success = bs.pop_byte(byte);
        read_bytes++;

        while (pop_byte_success && !byte.is_empty())
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

        if (read_bytes % step_size == 0)
        {
            uint8_t current_progress = static_cast<uint8_t>((read_bytes * 100) / total_bytes);

            // Only update terminal UI if percentage actually changed
            if (current_progress > last_progress)
            {
                Terminal::run_with_progress("Decoding Content", current_progress);
                last_progress = current_progress;
            }
        }
    }

    // Ensure terminal shows 100% when loop finishes
    Terminal::run_with_progress("Decoding Content", 100);
    std::cout << std::endl;

    return true;
}

bool Decoder::decode(std::string in_file, std::string &out_dir, std::string &out_path)
{
    bool success;
    if (!pretest(in_file, out_dir))
        return false;

    DecodedContent content = DecodedContent();
    std::array<TYPE_FREQUENCY, BYTE_SIZE> freq;
    freq.fill(0);

    if (!read_metadata(out_dir, out_path))
        return on_fail();
    if (!Utils::read_uint16_t(reader, content.num_unique_chars))
        return on_fail();
    if (!Utils::read_uint8_t(reader, content.max_freq_byte_size))
        return on_fail();
    if (!_read_chars(freq, content.num_unique_chars, content.max_freq_byte_size))
        return on_fail();
    if (!reader->next_byte(content.padding))
        return on_fail();

    Node *root = Utils::build_huffman_tree(freq);
    if (!root)
        return on_fail();

    ByteStream bs;
    _read_code_stream(bs);
    if (bs.size() == 0)
    {
        delete root;
        return on_fail();
    }

    bs.padding_back(content.padding);
    success = _decode(bs, root, content.content_codes);
    success = writer->write(content.content_codes, true);

    delete root;

    writer->close();
    reader->close();

    return success;
}